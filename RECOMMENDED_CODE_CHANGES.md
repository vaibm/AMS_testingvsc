# Recommended Code Change Snippets (L552_AMS)

This document collects **implementation-ready snippets** for the recommendations discussed:
- Non-blocking 400Hz loop (timer-flag driven)
- Stronger reinit/fault handling for PEC and false readings
- Dual CAN channel separation (logger + charger)
- Watchdog-friendly main loop structure

> These are **drop-in templates**; adapt names/IDs to your exact generated STM32Cube files.

---

## 1) 400Hz Non-Blocking Scheduler (Replace `HAL_Delay` Loop)

### 1.1 Global flags/state (add in a shared C file)

```c
#include <stdint.h>
#include <stdbool.h>

volatile uint8_t bms_tick_400hz = 0U;
volatile uint8_t can_log_tick = 0U;
volatile uint8_t charger_rx_pending = 0U;

volatile uint32_t g_uptime_ms = 0U;
```

### 1.2 Timer callback (2.5ms => 400Hz)

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        bms_tick_400hz = 1U;
    }

    if (htim->Instance == TIM2) {
        g_uptime_ms++;
    }

    if (htim->Instance == TIM5) {
        can_log_tick = 1U;
    }
}
```

### 1.3 Main loop (non-blocking)

```c
while (1)
{
    if (bms_tick_400hz != 0U) {
        bms_tick_400hz = 0U;

        adbms_readCell_loop();
        adbmsReinitMain();
    }

    if (charger_rx_pending != 0U) {
        charger_rx_pending = 0U;
        Charger_HandleRxFrame();
        Charger_SendResponse();
    }

    if (can_log_tick != 0U) {
        can_log_tick = 0U;
        canFraming();
        CAN_DataTX_1s();
    }

    Safety_BackgroundChecks();
    FeedWatchdogIfHealthy();
}
```

---

## 2) Robust Reinit/Fault Manager (PEC + False Reading Protection)

### 2.1 Fault state data structure

```c
typedef struct
{
    uint8_t pec_consecutive;
    uint8_t plausibility_consecutive;
    uint8_t reinit_attempts;
    uint32_t last_good_sample_ms;
    uint8_t hard_fault;
} bms_ic_fault_state_t;

static bms_ic_fault_state_t g_ic_fault[1];
```

### 2.2 Check all PEC sources (not just `stat_pec`)

```c
static uint8_t BMS_HasAnyPecError(const cell_asic *ic)
{
    return (uint8_t)(
        (ic->cccrc.cfgr_pec  != 0U) ||
        (ic->cccrc.cell_pec  != 0U) ||
        (ic->cccrc.acell_pec != 0U) ||
        (ic->cccrc.scell_pec != 0U) ||
        (ic->cccrc.fcell_pec != 0U) ||
        (ic->cccrc.aux_pec   != 0U) ||
        (ic->cccrc.raux_pec  != 0U) ||
        (ic->cccrc.stat_pec  != 0U) ||
        (ic->cccrc.comm_pec  != 0U) ||
        (ic->cccrc.pwm_pec   != 0U) ||
        (ic->cccrc.sid_pec   != 0U)
    );
}
```

### 2.3 Plausibility check for false readings

```c
static uint8_t BMS_IsCellDataPlausible(const SEG_PARAMS *seg, const SEG_PARAMS *prev)
{
    for (uint8_t i = 0U; i < 12U; i++) {
        float v = seg->CELL_V[i];
        float dv = v - prev->CELL_V[i];
        if (dv < 0.0f) { dv = -dv; }

        if ((v < 1.5f) || (v > 5.0f)) {
            return 0U;
        }

        if (dv > 0.20f) {
            return 0U;
        }
    }

    if ((seg->IC_T < -40.0f) || (seg->IC_T > 125.0f)) {
        return 0U;
    }

    return 1U;
}
```

### 2.4 Reinit state machine (bounded retries + safe-state)

```c
#define PEC_ERR_THRESHOLD            (3U)
#define PLAUSIBILITY_ERR_THRESHOLD   (3U)
#define MAX_REINIT_ATTEMPTS          (3U)
#define SAMPLE_TIMEOUT_MS            (15U)

static void BMS_EnterSafeState(void)
{
    AMS_SAFE = 0U;
    AMS_Charg_I = 0U;
    AMS_Charg_V = 0U;
}

static uint8_t BMS_VerifyAfterReinit(cell_asic *ic)
{
    adBmsWakeupIc(1U);
    adBmsReadData(1U, ic, RDCFGA, Config, A);

    if (BMS_HasAnyPecError(ic) != 0U) {
        return 0U;
    }

    if ((ic->cccrc.cmd_cntr == 0U) || (ic->cccrc.cmd_cntr == 0x3FU)) {
        return 0U;
    }

    return 1U;
}

void adbmsReinitMain(void)
{
    bms_ic_fault_state_t *fs = &g_ic_fault[0];

    if (BMS_HasAnyPecError(&IC[0]) != 0U) {
        if (fs->pec_consecutive < 255U) { fs->pec_consecutive++; }
    } else {
        fs->pec_consecutive = 0U;
    }

    if ((g_uptime_ms - fs->last_good_sample_ms) > SAMPLE_TIMEOUT_MS) {
        if (fs->plausibility_consecutive < 255U) { fs->plausibility_consecutive++; }
    }

    if ((fs->pec_consecutive >= PEC_ERR_THRESHOLD) ||
        (fs->plausibility_consecutive >= PLAUSIBILITY_ERR_THRESHOLD)) {

        if (fs->reinit_attempts < MAX_REINIT_ATTEMPTS) {
            fs->reinit_attempts++;
            adbms_init_loop();

            if (BMS_VerifyAfterReinit(&IC[0]) != 0U) {
                fs->pec_consecutive = 0U;
                fs->plausibility_consecutive = 0U;
                fs->reinit_attempts = 0U;
                return;
            }
        } else {
            fs->hard_fault = 1U;
            BMS_EnterSafeState();
        }
    }
}
```

---

## 3) Dual CAN Separation (Logger Bus + Charger Bus)

## 3.1 Conceptual channel split

```c
/* FDCAN1: logging bus */
extern FDCAN_HandleTypeDef hfdcan1;

/* FDCAN2: charger bus */
extern FDCAN_HandleTypeDef hfdcan2;
```

### 3.2 Start both channels + notifications

```c
void CAN_InitAll(void)
{
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) { Error_Handler(); }
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) { Error_Handler(); }

    if (HAL_FDCAN_ActivateNotification(&hfdcan2,
                                       FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                       0U) != HAL_OK) {
        Error_Handler();
    }
}
```

### 3.3 Charger RX ISR hook

```c
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((hfdcan->Instance == FDCAN2) &&
        ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U)) {
        charger_rx_pending = 1U;
    }
}
```

### 3.4 Charger frame processing template

```c
#define CAN_ID_CHARGER_CMD_START   (0x300U)
#define CAN_ID_CHARGER_CMD_STOP    (0x301U)
#define CAN_ID_BMS_CHARGER_STATUS  (0x201U)

static uint32_t charger_last_rx_ms = 0U;

void Charger_HandleRxFrame(void)
{
    FDCAN_RxHeaderTypeDef rxh;
    uint8_t data[8];

    if (HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &rxh, data) != HAL_OK) {
        return;
    }

    charger_last_rx_ms = g_uptime_ms;

    switch (rxh.Identifier) {
    case CAN_ID_CHARGER_CMD_START:
        AMS_Charg_I = (uint16_t)data[0] * 100U;
        AMS_Charg_V = (uint16_t)data[1] * 10U;
        break;

    case CAN_ID_CHARGER_CMD_STOP:
        AMS_Charg_I = 0U;
        break;

    default:
        break;
    }
}

void Charger_SendResponse(void)
{
    FDCAN_TxHeaderTypeDef txh;
    uint8_t tx[8] = {0};

    txh.Identifier = CAN_ID_BMS_CHARGER_STATUS;
    txh.IdType = FDCAN_STANDARD_ID;
    txh.TxFrameType = FDCAN_DATA_FRAME;
    txh.DataLength = FDCAN_DLC_BYTES_8;
    txh.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txh.BitRateSwitch = FDCAN_BRS_OFF;
    txh.FDFormat = FDCAN_FD_CAN;
    txh.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txh.MessageMarker = 0U;

    tx[0] = AMS_SAFE;
    tx[1] = AMS_SOC;
    tx[2] = (uint8_t)(AMS_Charg_I / 100U);
    tx[3] = (uint8_t)(AMS_Charg_V / 10U);

    (void)HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &txh, tx);
}
```

---

## 4) 400Hz Timing Notes (Practical)

- 400Hz means **2.5ms period**.
- Keep SPI transactions deterministic (avoid long blocking retries inside the cycle).
- Keep heavy formatting/logging out of 400Hz critical path.
- Prefer event flags + ISR wakeups instead of polling loops with delays.

Quick reference:

```text
Loop budget at 400Hz:
- Target period: 2.5 ms
- BMS read + parse + checks: <= 2.0 ms
- Margin: >= 0.5 ms
```

---

## 5) Optional Safety Hooks (Recommended)

### 5.1 Charger comms timeout safety

```c
#define CHARGER_TIMEOUT_MS  (1000U)

void Charger_TimeoutCheck(void)
{
    if ((g_uptime_ms - charger_last_rx_ms) > CHARGER_TIMEOUT_MS) {
        AMS_Charg_I = 0U;
        AMS_SAFE = 0U;
    }
}
```

### 5.2 Watchdog gate (feed only if healthy)

```c
void FeedWatchdogIfHealthy(void)
{
    if ((AMS_SAFE != 0U) && (g_ic_fault[0].hard_fault == 0U)) {
        HAL_IWDG_Refresh(&hiwdg);
    }
}
```

---

## 6) Integration Order (Suggested)

1. Remove `HAL_Delay(3)` and move to timer-flag scheduler.
2. Add full PEC checks + bounded reinit retries.
3. Add plausibility filters for false readings.
4. Add CAN channel split logic (or strict filter partitioning if single bus).
5. Add timeout and watchdog safety hooks.
6. Run long-duration soak and fault-injection tests before hardware release.

---

## 7) Validation Checklist

- [ ] 400Hz cycle measured with scope/logic pin (jitter within target)
- [ ] No blocking delay in control path
- [ ] PEC transient recovered without false shutdown
- [ ] Persistent PEC drives safe-state after max retries
- [ ] Charger timeout forces charge current to zero
- [ ] Watchdog is refreshed only when health conditions are valid
- [ ] CAN logger traffic does not delay charger command handling


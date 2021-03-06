//*****************************************************************************
//
// FILE:   main.c
//
// TITLE:  IFEC Project
//
// IFEC 2020 CU Boulder
//
//*****************************************************************************

#include <stdbool.h>
#include "config.h"

#include "driverlib.h"
#include "device.h"
#include "battery.h"
#include "src_adc.h"
#include "src_epwm.h"
#include "src_gpio.h"
#include "src_timers.h"

/** Controls */
#include "pid.h"
#include "mppt.h"

/** Test Selection **/
/*      NORMAL_OPERATION
 *      TEST_OUTPUT_BUCKS
 *      TEST_OUTPUT_BUCKS_OPEN_LOOP
 *      TEST_MPPT_BUCKS
 **/
#define NORMAL_OPERATION
//#define USE_WATCHDOG
//#define USE_CC_CV
//#define TEST_MPPT_BUCKS


/** Global Variables */
Battery_t battery = {
         .voltage = 0.0,
         .current = 0.0,
         .state = Supply,
         .charger = Charging_Inactive
};


PID_t five_volt_buck_pid;
PID_t three_volt_buck_pid;
MPPT_t mppt_one;
MPPT_t mppt_two;


void main(void) {
    // Initialize device clock and peripherals
    Device_init();
    Device_initGPIO();

    // PID
    PID_init(&five_volt_buck_pid, KP, KI, KD, V_BUCK_5V_REF, PID_US);
    PID_init(&three_volt_buck_pid, KP, KI, KD, V_BUCK_3V3_REF, PID_US);

    // MPPT
    mppt_init(&mppt_one, MPPT_ONE_ID, MPPT_1_DELTA_DC, MPPT_1_DELTA_DC_MAX);
    mppt_init(&mppt_two, MPPT_TWO_ID, MPPT_2_DELTA_DC, MPPT_2_DELTA_DC_MAX);

    // Battery
    init_battery(&battery);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();

    // Configure peripherals
    init_led5();
    init_adc();

    initEPWMGPIO();
    initEPWM(BUCK_5V_PWM);
    initEPWM(BUCK_3V3_PWM);
    initEPWM(MPPT_1_PWM);
    initEPWM(MPPT_2_PWM);

    init_timer(PID_TIMER, PID_US);
    init_timer(MPPT_TIMER, TIMER_500US);

    Interrupt_register(PID_TIMER_INT, &PID_Timer_ISR);
    Interrupt_register(MPPT_TIMER_INT, &MPPT_Timer_ISR);

    // Enable interrupts
    Interrupt_enable(PID_TIMER_INT);
    Interrupt_enable(MPPT_TIMER_INT);

    CPUTimer_startTimer(PID_TIMER);
    CPUTimer_startTimer(MPPT_TIMER);

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

#ifdef USE_WATCHDOG
    // Watchdog will trigger after 1.3ms of inactivity
    SysCtl_setWatchdogMode(SYSCTL_WD_MODE_RESET);
    SysCtl_setWatchdogPredivider(SYSCTL_WD_PREDIV_4096);
    SysCtl_setWatchdogPrescaler(SYSCTL_WD_PRESCALE_32);
    SysCtl_serviceWatchdog();
    SysCtl_enableWatchdog();
#else
    SysCtl_disableWatchdog();
#endif


    for(;;) {
#ifdef NORMAL_OPERATION
           /*
            * When enabled, the watchdog makes sure both control loops
            *   run at least every 1.3ms otherwise the device will restart
            */
   //        SysCtl_serviceWatchdog();
           /* Only used for inital testing
           change_pwm_duty_cycle(MPPT_1_PWM, 50.0);
           change_pwm_duty_cycle(MPPT_2_PWM, 50.0);

           change_pwm_duty_cycle(BUCK_5V_PWM, 50.0);
           change_pwm_duty_cycle(BUCK_3V3_PWM, 50.0);
           */
           if(get_pid_active() == true) {
                   // get adc conversions
               update_output_buck_conversions();
               //change_pwm_duty_cycle(BUCK_5V_PWM, PID_calculate(&five_volt_buck_pid, get_buck_stepped_down_v(BUCK_5V_ID)));
               //change_pwm_duty_cycle(BUCK_5V_PWM, PID_calculate(&three_volt_buck_pid, get_buck_stepped_down_v(BUCK_5V_ID)));
               //change_pwm_duty_cycle(BUCK_3V3_PWM, PID_calculate(&three_volt_buck_pid, get_buck_stepped_down_v(BUCK_3V3_ID)));
               change_pwm_duty_cycle(BUCK_5V_PWM, PID_calculate(&five_volt_buck_pid, get_buck_v(BUCK_5V_ID)));
               change_pwm_duty_cycle(BUCK_3V3_PWM, PID_calculate(&three_volt_buck_pid, get_buck_v(BUCK_3V3_ID)));
               set_pid_active(false);
           }

           /** MPPT **/
           if(get_mppt_active() == true) {
               // wait for all MPPT and Battery ADC conversions to be done
               update_mppt_conversions();
               update_battery_conversions();

               /** Update values **/
               mppt_update_values(&mppt_one);
               //mppt_update_values(&mppt_two);
               //update_battery(&battery);

               /** Change duty cycles **/
               change_pwm_duty_cycle(MPPT_1_PWM, (get_duty_cycle(MPPT_1_PWM) + mppt_calculate(&mppt_one)));
               change_pwm_duty_cycle(MPPT_2_PWM, (get_duty_cycle(MPPT_2_PWM) + mppt_calculate(&mppt_two)));

               // TODO: add hysteresis on which PV to use?
               // Run MPPT on PV with the highest voltage

               set_mppt_active(false);
           }
           else if ((get_pid_active() == false) && (get_mppt_active() == false)) {
               // go to low-power mode until a timer interrupt wakes up CPU
               IDLE;
           }

#endif
#ifdef TEST_OUTPUT_BUCKS
        if(get_pid_active() == true) {
            update_output_buck_conversions();
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&five_volt_buck_pid, get_buck_v(BUCK3V3_BASE)));
            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&three_volt_buck_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
        }
        else if (get_pid_active() == false) {
            // go to low-power mode until a timer interrupt wakes up CPU
            IDLE;
        }
#endif
#ifdef TEST_MPPT_BUCKS
        if(get_mppt_active() == true) {
            update_mppt_conversions();
            update_battery_conversions();

            /** Update values **/
            mppt_update_values(&mppt_one);
            //mppt_update_values(&mppt_two);

            /** Change duty cycles **/
            change_pwm_duty_cycle(MPPT_1_PWM, (get_duty_cycle(MPPT_1_PWM) + mppt_calculate(&mppt_one)));
            //change_pwm_duty_cycle(MPPT_2_PWM, (get_duty_cycle(MPPT_2_PWM) + mppt_calculate(&mppt_two)));

            set_mppt_active(false);
        }
        else if (get_mppt_active() == false) {
            // go to low-power mode until a timer interrupt wakes up CPU

            IDLE;
        }
#endif
       }
   }

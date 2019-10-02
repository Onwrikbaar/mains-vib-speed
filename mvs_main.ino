/*
 * HitaSpeed -- Speed controller for oldfashioned vibrators with a mains-voltage AC motor.
 * (C) 2016 - 2019 Mark Onwrikbaar <onwrikbaar@hotmail.com>
 */

#include <avr/wdt.h>

#define CURRENT_THR_PIN         PD2             // Not used yet.
#define VOLTAGE_THR_PIN         PD3             // From the collector of the opto-isolator that watches the mains voltage.
#define TRIAC_TRIGGER_PIN       PD4             // To trigger the power photo triac (active low).
#define DETECT_OUT_PIN          PD7             // Just for debugging, to trigger an oscilloscope.

static const char versionString[] = "# HitaSpeed v0.33";

// Currently a 50 Hz sine is assumed. TODO Detect the actual frequency automatically.
static float avgAboveThMics = 9600.0;
static float avgBelowThMics =  150.0;
static const unsigned int halfPeriodMics = 10000U;
static volatile int triacTriggerHoldoffMics = halfPeriodMics;

/**
 * Reset the trigger holdoff delay to its maximum value.
 */
static inline void softStart(void)
{
    triacTriggerHoldoffMics = halfPeriodMics;
}

/**
 * On each call, this function returns the next number of microseconds
 * to wait between the zero crossing and triggering the triac.
 * 
 * In a real application, this function would check a variable that might be set by a potmeter or some sensor,
 * or even remotely across the internet, and adjust the triac trigger holdoff delay accordingly.
 */
static unsigned int triggerDelay(void)
{
    const unsigned int minDelayMics = 200;
    const unsigned int maxDelayMics = halfPeriodMics - 1000;
    static int stepMics = -17;

    if ((triacTriggerHoldoffMics += stepMics) <= minDelayMics) {
        triacTriggerHoldoffMics = minDelayMics;
        stepMics = -stepMics;                   // Reverse direction.
    } else if (triacTriggerHoldoffMics >= maxDelayMics) {
        triacTriggerHoldoffMics = maxDelayMics;
        stepMics = -stepMics;                   // Reverse direction.
    }
    return triacTriggerHoldoffMics;
}

/**
 * This interrupt routine triggers the triac. It is invoked when timer 1 expires.
 */
ISR(TIMER1_COMPA_vect)
{
    digitalWrite(TRIAC_TRIGGER_PIN, LOW);
    TCCR1A = 0;                                 // Set entire TCCR1A register to 0.
    TCCR1B = 0;                                 // Same for TCCR1B -> stop timer.
    delayMicroseconds(40);                      // The triac trigger must have a minimum width.
    digitalWrite(TRIAC_TRIGGER_PIN, HIGH);      // Remove the trigger again.
}

/**
 * Set up a timer interrupt to deliver the trigger to the triac at the desired time.
 * The 16 MHz Arduino clock is prescaled to 250 kHz, i.e. a period of 4 µs;
 * hence the requested delay in microseconds must be divided by 4.
 */
static void scheduleTriacTrigger(unsigned int micsSinceZeroCross, unsigned int delayMics)
{
    if (delayMics > (halfPeriodMics - halfPeriodMics / 32)) {
        return;                                 // Don't bother triggering past circa 97% of the half sine wave's period.
    }
    TCNT1 = 0;                                  // Start counting at 0.
    TCCR1A = 0;
    const uint16_t ticks = (delayMics > micsSinceZeroCross ? (delayMics - micsSinceZeroCross) / 4 : 0);
    OCR1A = (ticks < 4 ? 4 : ticks);            // Set the compare match A register. Use a minimum value to ensure the interrupt.
    TCCR1B = (1<<CS10) | (1<<CS11) | (1<<WGM12);// Select divide by 64 prescaler and turn on CTC mode.
}

/**
 * TODO Implement when the electronics support it.
 * When controlling inductive or capacitive loads, checking the current zero crossing too
 * allows smoother regulation as well as a better soft start.
 */
static void currentCrossedThreshold()
{
    const auto nowMics = micros();
    // TODO Implement.
}

/**
 * This function is called whenever the mains voltage crosses the detection threshold,
 * which happens twice during each half cycle (before and after the zero crossing),
 * i.e. four times for each full cycle.
 * When the voltage rises above the threshold, we have just passed a zero crossing,
 * which we use as a reference for the triac trigger timing.
 */
static void voltageCrossedThreshold()
{
    static unsigned long tsLastAboveThMics = 0UL;
    static unsigned long tsLastBelowThMics = 0UL;

    const auto nowMics = micros();              // Get the timestamp.
    if (digitalRead(VOLTAGE_THR_PIN) == LOW) {  // Mains voltage rose above detection threshold.
        digitalWrite(DETECT_OUT_PIN, HIGH);
        const auto deltaMics = (tsLastAboveThMics = nowMics) - tsLastBelowThMics;
        if (deltaMics > halfPeriodMics) {       // Mains voltage was just switched on (again).
            softStart();
        } else {                                // Update the moving average, and schedule trigger.
            avgBelowThMics += ((float)deltaMics - avgBelowThMics) / 32.0;
            scheduleTriacTrigger((unsigned int)((avgBelowThMics + 1.0) / 2), triggerDelay());
        }
    } else {                                    // Mains voltage fell below detection threshold.
        digitalWrite(DETECT_OUT_PIN, LOW);
        const auto deltaMics = (tsLastBelowThMics = nowMics) - tsLastAboveThMics;
        if (deltaMics > halfPeriodMics) {       // Mains voltage was just switched on (again).
            softStart();
        } else {                                // Update the moving average.
            avgAboveThMics += ((float)deltaMics - avgAboveThMics) / 32.0;
        }
    }
}

/**
 * Timer 1 is used for the triac trigger holdoff.
 */
static void setupTimer1()
{
    TCCR1A = 0;                                 // Set entire TCCR1A register to 0.
    TCCR1B = 0;                                 // Same for TCCR1B -> stop timer.
    TIMSK1 |= (1 << OCIE1A);                    // Enable timer compare interrupt.
}


void setup()
{
    noInterrupts();
    digitalWrite(TRIAC_TRIGGER_PIN, HIGH);
    pinMode(TRIAC_TRIGGER_PIN, OUTPUT);
    pinMode(CURRENT_THR_PIN, INPUT_PULLUP);
    pinMode(VOLTAGE_THR_PIN, INPUT_PULLUP);
    pinMode(DETECT_OUT_PIN, OUTPUT);            // For debugging the hardware.
    pinMode(LED_BUILTIN, OUTPUT);               // The LED is not used yet.
    Serial.begin(9600);
    Serial.println(versionString);
    attachInterrupt(digitalPinToInterrupt(CURRENT_THR_PIN), currentCrossedThreshold, CHANGE);
    attachInterrupt(digitalPinToInterrupt(VOLTAGE_THR_PIN), voltageCrossedThreshold, CHANGE);
    setupTimer1();
//  wdt_enable(WDTO_1S);                        // Set watchdog timer to 1 second.
    interrupts();
}


void loop()
{
    wdt_reset();                                // Keep our watchdog happy.
}

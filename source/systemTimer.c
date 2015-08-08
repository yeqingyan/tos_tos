/*
 * GetSystemTimerBase
 * ------------------
 * return timer base address.
 */
int GetSystemTimerBase(){
        return 0x20003000;
}

/*
 * GetTimeStamp
 * ------------
 *  return lower 32 bits ticks
 */
int GetTimeStamp() {
        int *timerBase = (int *)GetSystemTimerBase();
        return *(timerBase+1);
}

/*
 * Wait
 * ----
 *  wait n ticks.
 *
 *  Paramters:
 *  n: ticks number.
 */
void Wait(int n){
        int start, end;

        start = GetTimeStamp();
        
        /* Loop if time laspe is lower than wait time */
        while(1){
                end = GetTimeStamp();
                if ((end - start) > n) { 
                        break;
                }
        }
        
}

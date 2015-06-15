#ifndef bsp_h
#define bsp_h

#define BSP_TICKS_PER_SEC   100U

void BSP_init(void);
void BSP_showState(char_t const *state);
void BSP_terminate(int16_t result);

#endif // bsp_h

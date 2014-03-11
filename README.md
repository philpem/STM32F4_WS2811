WS281x driver for STM32F4 Discovery Board
=========================================

This is a variant of [Elias's STM32F2 WS2812 driver](http://eliaselectronics.com/driving-a-ws2812-rgb-led-with-an-stm32/) which has been ported to run on the STM32F4 Discovery Board as a proof of concept.

Timing has been tested on a WS2811B (which requires a longer "T1H" high time than the 281x and 281xS) but not any other WS281x series devices. In theory it should work with the standard WS281x and WS281xS.

Hook up your WS2811B thusly:

  * Vcc and ground as normal (WS2811s will generally work from 3.3V or 5V but are usually specified for 5V).
  * PB4 (GPIOB bit 4) to the WS281x's DI (Data In) pin.

Things I know are icky:

  * Timings are probably suboptimal. [Tim wrote a nice article on the timing requirements](http://cpldcpu.wordpress.com/2014/01/14/light_ws2812-library-v2-0-part-i-understanding-the-ws2812/) which includes timing constraints.
  * Timer clock is obtained by dividing the system clock (obtained from RCC\_GetClocksFreq) by a fixed value. This fixed value depends on the APB divider... We really need to know what that is. Look for the comment "Compute the prescaler value".
  * <del>There's a bodge in place to try and prevent the TIM1 timer from releasing a random-length high pulse (essentially a glitch) when the timer is initialised. Look for the comment "PAP: Start up TIM1 so that the PWM output is forced low (it starts out high)". I'm open to suggestions on ways to improve this, if anyone can think of one.</del> Fixed! Thanks to Zyp on ##stm32@irc.freenode.net for explaining how to fix this.
  * The colour wheel is a bit nasty and doesn't really sequence cleanly. It'd be better (much cleaner code) if this were generated on the fly.
  * The cycle-spinner delay routine is icky. We've got the SYSTICK timer, we really should use it...
  * The LED assignments (''pix\_map'' in the WS2812\_send routine) are for my hand-soldered WS2812-board-and-an-RGB-LED thing. They probably aren't correct for the WS2812 self-contained controller-and-LED chip.
  * Memory usage is pretty phenomenal. 24 16-bit words for every LED, plus 42 trailing words. That's 132 bytes just for one LED...


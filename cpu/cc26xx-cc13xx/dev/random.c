/*
 * Copyright (c) 2015, Mike Snyder - <mike206@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Random number generator routines exploiting the cc26xx hardware
 *         capabilities.
 *
 *         This file overrides core/lib/random.c.
 *
 * \author
 *         Mike Snyder - <mike206@gmail.com>
 */
#include <stdint.h>
#include "ti-lib.h"
/*******************************************************************************
 * GLOBAL VARIABLES
 */
static uint32_t lastTrngVal;
/*---------------------------------------------------------------------------*/
/**
 * \brief      Seed the cc26xx random number generator.
 * \param seed Ignored. It's here because the function prototype is in core.
 */
void
random_init(unsigned short seed)
{
  // configure TRNG
  // Note: Min=4x64, Max=1x256, ClkDiv=1+1 gives the same startup and refill
  //       time, and takes about 11us (~14us with overhead).
  ti_lib_trng_configure( 256, 256, 0x01 );

  // enable TRNG
  ti_lib_trng_enable();

  // init variable to hold the last value read
  lastTrngVal = 0;

  return;

}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Generates a new random number using the cc26xx RNG.
 * \return     The random number.
 */
unsigned short
random_rand(void)
{
  uint32_t trngVal;

  // initialize and enable TRNG if TRNG is not enabled
  if (0 == (HWREG(TRNG_BASE + TRNG_O_CTL) & TRNG_CTL_TRNG_EN))
  {
    random_init(0);
  }

  // check that a valid value is ready
  while(!(ti_lib_trng_status_get() & TRNG_NUMBER_READY));

  // check to be sure we're not getting the same value repeatedly
  if ( (trngVal = ti_lib_trng_number_get(TRNG_LOW_WORD)) == lastTrngVal )
  {
    return( 0xDEAD );
  }
  else // value changed!
  {
    // so save last TRNG value
    lastTrngVal = trngVal;

    return( trngVal );
  }

}

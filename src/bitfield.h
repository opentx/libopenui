/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include <inttypes.h>

// A set of bitfield handling macros

template <typename T>
inline T bfBit(uint8_t n)
{
  return T(1) << n;
}

#define BFBIT_FLIP(y, mask)       ( y ^=  (mask) )

//! Create a bitmask of length 'len'.
template <typename T>
inline T bfBitmask(uint8_t len)
{
  return bfBit<T>(len) - 1;
}

//! Create a bitfield mask of length 'len' starting at bit 'start'.
template <typename T>
inline T bfMask(uint8_t start, uint8_t len)
{
  return bfBitmask<T>(len) << start;
}

//! Prepare a bitmask for insertion or combining.
template <typename T>
inline T bfPrep(T x, uint8_t start, uint8_t len)
{
  return (x & bfBitmask<T>(len)) << start;
}

//! Extract a bitfield of length 'len' starting at bit 'start' from  'y'.
template <typename T>
inline T bfGet(T y, uint8_t start, uint8_t len)
{
  return ((y)>>(start)) & bfBitmask<T>(len);
}

//! Insert 'len' bits of 'x 'into 'y', starting at bit 'start' from  'y'.
template <class T>
inline T bfSet(T to, T from, uint8_t start, uint8_t len)
{
  return (to & ~bfMask<T>(start, len)) | bfPrep<T>(from, start, len);
}

template <class T>
inline T bfBitGet(T y, T mask)
{
  return (y & mask);
}

template <class T>
inline T bfBitSet(T y, T mask)
{
  return (y | mask);
}

template <class T>
inline T bfBitToggle(T y, T mask)
{
  return (y ^ mask);
}

template <class T>
inline T bfSingleBitGet(T y, uint8_t i)
{
  return bfBitGet(y, bfBit<T>(i));
}

template <class T>
inline T bfSingleBitSet(T y, uint8_t i)
{
  return bfBitSet(y, bfBit<T>(i));
}

template <class T>
inline T bfSingleBitToggle(T y, uint8_t i)
{
  return bfBitToggle(y, bfBit<T>(i));
}

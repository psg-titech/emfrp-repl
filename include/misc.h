/** -------------------------------------------
 * @file   misc.h
 * @brief  Miscellaneous
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */
#pragma once

#if __STD_VERSION__ <= 201710L
// nullptr is appeared in C23.
#define nullptr NULL
#endif

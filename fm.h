/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FM_H
#define _FM_H

#if __cplusplus
extern "c" {
#endif

int fm_exists();

int fm_set_freq(int freq);

int fm_get_freq();

int fm_on();

int fm_off();

int fm_get_vol();

int fm_set_vol(int vol);
#if __cplusplus
};  // extern "C"
#endif

#endif  // _FM_H

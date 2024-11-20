/**************************************************************************//**
 *
 * @file display.h
 *
 * @author Christopher A. Bohn
 *
 * @brief Functions to display text on SSD1306 display module using either
 *      OneBitDisplay library or Adafruit_SSD1306 library.
 *
 * These functions provide a way to display strings on SSD1306 displays before
 * SSD1306 file streams are implemented in the CowPi_stdio library. There are
 * also functions to display diagnostic information about the compiler,
 * libraries, and application code.
 *
 ******************************************************************************/

/* SSD1306 display functions (c) 2024 Christopher A. Bohn
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COWPI_DISPLAY_H
#define COWPI_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the SSD1306 display module.
 *
 * Typically, the display module will be configured for eight rows of text.
 * If the number of columns of text is 10 or fewer, however, then the display
 * module will be configured for four rows of text.
 *
 * @param number_of_columns The number of character columns to be used.
 *      Valid values are 8, 10, 16, and 21.
 */
void initialize_display(int number_of_columns);

/**
 * Clears the contents of the SSD1306 display module, giving it a blank screen.
 */
void clear_display(void);

/**
 * Draws the CowPi logo on the SSD1306 display module.
 */
void draw_logo();

/**
 * Places the string on the specified row.
 * If the last character of the string (immediately before the terminal NUL) is
 * a newline character (ASCII 0x0A) then the display will be refreshed with this
 * string and any other buffered strings. Otherwise, this string will be
 * buffered until the next display refresh.
 *
 * @param row The row on which the string should be placed
 *      (0-7, with row 0 at the top)
 * @param string The NUL-terminated string to be displayed
 */
void display_string(int row, char const string[]);

/**
 * Updates the display with any buffered strings.
 */
void refresh_display(void);

/**
 * Prints the gcc, CowPi, and CowPi_stdio versions. Prints the core library
 * backing the Arduino framework, and the library used to drive the SSD1306
 * display module.
 *
 * Output is to both the display module and the Serial Monitor.
 */
void print_versions(void);

/**
 * Create a record of the build timestamp for the file that calls this function.
 *
 * @see print_build_timestamps()
 *
 * @param filename The name of the file calling this function.
 *      Use <code>__FILE__</code>.
 * @param date The date the file was compiled. Use <code>__DATE__</code>.
 * @param time The time the file was compiled. Use <code>__TIME__</code>.
 */
void record_build_timestamp(char const filename[], char const date[], char const time[]);

/**
 * Prints the record(s) of build timestamps.
 * If `only_most_recent` is true, then prints the time of the most
 * recently-built file. Otherwise, prints the build times of all files that
 * recorded their build times, up to the limit of what can be displayed.
 *
 * Output is to both the display module and the Serial Monitor.
 *
 * This is useful to double-check that you have the most-recent version of
 * your code running.
 *
 * @see record_build_timestamp()
 *
 * @param only_most_recent
 */
void print_build_timestamps(bool only_most_recent);

/**
 * Displays an 8-bit counter in the rightmost two columns of the specified row.
 * The counter increments each time the function is called, providing a
 * demonstration of liveness, or merely a demonstration that a code segment has
 * executed.
 *
 * @param row The display row on which to show the counter
 */
void count_visits(int row);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //COWPI_DISPLAY_H

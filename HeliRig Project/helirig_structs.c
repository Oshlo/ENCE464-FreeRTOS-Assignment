/*******************************************************
 * queue_structs.c
 *
 * TODO: Define this
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/

#include <stdint.h>
#include <stdbool.h>

// Structure used by the OLED display
typedef struct OLED_Message
{
    char strBuf[17];  // Shows what string to display on the OLED
    uint8_t charLine;  // Shows what line to display the string on
    uint8_t charPos;  //Show what character the string starts on
} OLEDMessage;

// Structure used by the Quad
typedef struct Quad_Type
{
    uint8_t A;
    uint8_t B;
    uint8_t L;
    uint8_t R;
    uint8_t Ap;
    uint8_t Bp;
    int32_t sum;

} QuadType;

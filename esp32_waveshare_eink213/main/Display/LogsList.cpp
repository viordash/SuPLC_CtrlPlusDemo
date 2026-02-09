#include "Display/LogsList.h"
#include "Display/display.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogsList::LogsList(const char *title) : ListBox(title) {
    curr_line = 0;
}

void LogsList::Append(const char *message) {
    char line[line_size];
    size_t len = strlen(message);

    if (len > line_size - 1) {
        const int elips_len = 3;
        strncpy(line, message, line_size - elips_len - 1);
        strcpy(&line[line_size - elips_len - 1], "...");
    } else {
        strcpy(line, message);
    }

    if (curr_line < LisBoxLinesCount) {
        strcpy(lines[curr_line], line);
        curr_line++;
    } else {
        for (size_t i = 0; i < LisBoxLinesCount - 1; i++) {
            strncpy(lines[i], lines[i + 1], sizeof(lines[0]));
        }
        strcpy(lines[LisBoxLinesCount - 1], line);
    }
    frame_buffer_req_render = true;
}
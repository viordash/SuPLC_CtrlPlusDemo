#pragma once

#include "board.h"
#include <errno.h>
#include <stdint.h>

/**
 * Drawing color
 */
typedef enum {
    COLOR_BLACK = 0, //!< Black (pixel on)
    COLOR_WHITE = 1, //!< White (pixel off)
    COLOR_GRAY = 2,  //!< Gray (pixel on-off)
} pixel_color_t;

/**
 * Default init for Drawing
 * @param dev Pointer to device descriptor
 * @return Non-zero if error occurred
 */
int drawing_init();

/**
 * Draw one pixel
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color of the pixel
 * @return Non-zero if error occurred
 */
int drawing_draw_pixel(FrameBuffer *fb, uint16_t x, uint16_t y, pixel_color_t color);

/**
 * Draw a horizontal line
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x X coordinate or starting (left) point
 * @param y Y coordinate or starting (left) point
 * @param w Line width
 * @param color Color of the line
 * @return Non-zero if error occurred
 */
int drawing_draw_hline(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t w, pixel_color_t color);

/**
 * Draw a vertical line
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x X coordinate or starting (top) point
 * @param y Y coordinate or starting (top) point
 * @param h Line height
 * @param color Color of the line
 * @return Non-zero if error occurred
 */
int drawing_draw_vline(FrameBuffer *fb, uint16_t x, uint16_t y, uint16_t h, pixel_color_t color);

/**
 * Draw a line
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 First x point coordinate
 * @param y0 First y point coordinate
 * @param x1 Second x point coordinate
 * @param y1 Second y point coordinate
 * @param color Color of the line
 * @return Non-zero if error occurred
 */
int drawing_draw_line(FrameBuffer *fb,
                      int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      pixel_color_t color);

/**
 * Draw a rectangle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x X coordinate or starting (top left) point
 * @param y Y coordinate or starting (top left) point
 * @param w Rectangle width
 * @param h Rectangle height
 * @param color Color of the rectangle border
 * @return Non-zero if error occurred
 */
int drawing_draw_rectangle(FrameBuffer *fb,
                           uint16_t x,
                           uint16_t y,
                           uint16_t w,
                           uint16_t h,
                           pixel_color_t color);

/**
 * Draw a filled rectangle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x X coordinate or starting (top left) point
 * @param y Y coordinate or starting (top left) point
 * @param w Rectangle width
 * @param h Rectangle height
 * @param color Color of the rectangle
 * @return Non-zero if error occurred
 */
int drawing_fill_rectangle(FrameBuffer *fb,
                           uint16_t x,
                           uint16_t y,
                           uint16_t w,
                           uint16_t h,
                           pixel_color_t color);

/**
 * Draw a circle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 X coordinate or center
 * @param y0 Y coordinate or center
 * @param r Radius
 * @param color Color of the circle border
 * @return Non-zero if error occurred
 */
int drawing_draw_circle(FrameBuffer *fb, uint16_t x0, uint16_t y0, uint8_t r, pixel_color_t color);

/**
 * Draw a filled circle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 X coordinate or center
 * @param y0 Y coordinate or center
 * @param r Radius
 * @param color Color of the circle
 * @return Non-zero if error occurred
 */
int drawing_fill_circle(FrameBuffer *fb, uint16_t x0, uint16_t y0, uint8_t r, pixel_color_t color);

/**
 * Draw a triangle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 First x point coordinate
 * @param y0 First y point coordinate
 * @param x1 Second x point coordinate
 * @param y1 Second y point coordinate
 * @param x2 third x point coordinate
 * @param y2 third y point coordinate
 * @param color Color of the triangle border
 * @return Non-zero if error occurred
 */
int drawing_draw_triangle(FrameBuffer *fb,
                          int16_t x0,
                          int16_t y0,
                          int16_t x1,
                          int16_t y1,
                          int16_t x2,
                          int16_t y2,
                          pixel_color_t color);

/**
 * Draw a filled triangle
 * @param dev Pointer to device descriptor
 * @param fb Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 First x point coordinate
 * @param y0 First y point coordinate
 * @param x1 Second x point coordinate
 * @param y1 Second y point coordinate
 * @param x2 third x point coordinate
 * @param y2 third y point coordinate
 * @param color Color of the triangle
 * @return Non-zero if error occurred
 */
int drawing_fill_triangle(FrameBuffer *fb,
                          int16_t x0,
                          int16_t y0,
                          int16_t x1,
                          int16_t y1,
                          int16_t x2,
                          int16_t y2,
                          pixel_color_t color);

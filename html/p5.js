/*! p5.js v0.5.12 August 11, 2017 */
/* This is a manually cut-down version for ExpressLRS use,
   We use this older version of p5 because it's smaller and it works for our use.
 */
(function(f){if(typeof exports==="object"&&typeof module!=="undefined"){module.exports=f()}else if(typeof define==="function"&&define.amd){define([],f)}else{var g;if(typeof window!=="undefined"){g=window}else if(typeof global!=="undefined"){g=global}else if(typeof self!=="undefined"){g=self}else{g=this}g.p5 = f()}})(function(){var define,module,exports;return (function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})
({45:[function(_dereq_,module,exports){

'use strict';

var p5 = _dereq_('./core/core');
_dereq_('./color/p5.Color');
_dereq_('./core/p5.Element');
_dereq_('./core/p5.Graphics');
_dereq_('./core/p5.Renderer2D');

_dereq_('./image/p5.Image');
_dereq_('./math/p5.Vector');

_dereq_('./color/creating_reading');
_dereq_('./color/setting');
_dereq_('./core/constants');
_dereq_('./core/environment');
_dereq_('./image/image');
_dereq_('./image/loading_displaying');
_dereq_('./image/pixels');
_dereq_('./io/files');
_dereq_('./math/math');
_dereq_('./math/calculation');
_dereq_('./math/trigonometry');
_dereq_('./core/rendering');
_dereq_('./core/2d_primitives');

_dereq_('./core/attributes');
_dereq_('./core/curves');
_dereq_('./core/vertex');
_dereq_('./core/structure');
_dereq_('./core/transform');

_dereq_('./webgl/p5.RendererGL');
_dereq_('./webgl/p5.Geometry');
_dereq_('./webgl/p5.RendererGL.Retained');
_dereq_('./webgl/p5.RendererGL.Immediate');
_dereq_('./webgl/primitives');
_dereq_('./webgl/loading');
_dereq_('./webgl/p5.Matrix');
_dereq_('./webgl/material');
_dereq_('./webgl/light');
_dereq_('./webgl/shader');
_dereq_('./webgl/camera');
_dereq_('./webgl/interaction');

_dereq_('./core/init.js');


module.exports = p5;

},{"./color/creating_reading":47,"./color/p5.Color":48,"./color/setting":49,"./core/2d_primitives":50,"./core/attributes":51,"./core/constants":53,"./core/core":54,"./core/curves":55,"./core/environment":56,"./core/init.js":58,"./core/p5.Element":59,"./core/p5.Graphics":60,"./core/p5.Renderer2D":62,"./core/rendering":63,"./core/structure":65,"./core/transform":66,"./core/vertex":67,"./image/image":74,"./image/loading_displaying":75,"./image/p5.Image":76,"./image/pixels":77,"./io/files":78,"./math/calculation":82,"./math/math":83,"./math/p5.Vector":85,"./math/trigonometry":88,"./webgl/camera":96,"./webgl/interaction":97,"./webgl/light":98,"./webgl/loading":99,"./webgl/material":100,"./webgl/p5.Geometry":101,"./webgl/p5.Matrix":102,"./webgl/p5.RendererGL":105,"./webgl/p5.RendererGL.Immediate":103,"./webgl/p5.RendererGL.Retained":104,"./webgl/primitives":106,"./webgl/shader":107}],46:[function(_dereq_,module,exports){
/**
 * @module Conversion
 * @submodule Color Conversion
 * @for p5
 * @requires core
 */

'use strict';

/**
 * Conversions adapted from <http://www.easyrgb.com/math.html>.
 *
 * In these functions, hue is always in the range [0,1); all other components
 * are in the range [0,1]. 'Brightness' and 'value' are used interchangeably.
 */

var p5 = _dereq_('../core/core');
p5.ColorConversion = {};

/**
 * Convert an HSBA array to HSLA.
 */
p5.ColorConversion._hsbaToHSLA = function(hsba) {
  var hue = hsba[0];
  var sat = hsba[1];
  var val = hsba[2];

  // Calculate lightness.
  var li = (2 - sat) * val / 2;

  // Convert saturation.
  if (li !== 0) {
    if (li === 1) {
      sat = 0;
    } else if (li < 0.5) {
      sat = sat / (2 - sat);
    } else {
      sat = sat * val / (2 - li * 2);
    }
  }

  // Hue and alpha stay the same.
  return [hue, sat, li, hsba[3]];
};

/**
 * Convert an HSBA array to RGBA.
 */
p5.ColorConversion._hsbaToRGBA = function(hsba) {
  var hue = hsba[0] * 6;  // We will split hue into 6 sectors.
  var sat = hsba[1];
  var val = hsba[2];

  var RGBA = [];

  if (sat === 0) {
    RGBA = [val, val, val, hsba[3]];  // Return early if grayscale.
  } else {
    var sector = Math.floor(hue);
    var tint1 = val * (1 - sat);
    var tint2 = val * (1 - sat * (hue - sector));
    var tint3 = val * (1 - sat * (1 + sector - hue));
    var red, green, blue;
    if (sector === 1) {  // Yellow to green.
      red = tint2;
      green = val;
      blue = tint1;
    } else if (sector === 2) {  // Green to cyan.
      red = tint1;
      green = val;
      blue = tint3;
    } else if (sector === 3) {  // Cyan to blue.
      red = tint1;
      green = tint2;
      blue = val;
    } else if (sector === 4) {  // Blue to magenta.
      red = tint3;
      green = tint1;
      blue = val;
    } else if (sector === 5) {  // Magenta to red.
      red = val;
      green = tint1;
      blue = tint2;
    } else {  // Red to yellow (sector could be 0 or 6).
      red = val;
      green = tint3;
      blue = tint1;
    }
    RGBA = [red, green, blue, hsba[3]];
  }

  return RGBA;
};

/**
 * Convert an HSLA array to HSBA.
 */
p5.ColorConversion._hslaToHSBA = function(hsla) {
  var hue = hsla[0];
  var sat = hsla[1];
  var li = hsla[2];

  // Calculate brightness.
  var val;
  if (li < 0.5) {
    val = (1 + sat) * li;
  } else {
    val = li + sat - li * sat;
  }

  // Convert saturation.
  sat = 2 * (val - li) / val;

  // Hue and alpha stay the same.
  return [hue, sat, val, hsla[3]];
};

/**
 * Convert an HSLA array to RGBA.
 *
 * We need to change basis from HSLA to something that can be more easily be
 * projected onto RGBA. We will choose hue and brightness as our first two
 * components, and pick a convenient third one ('zest') so that we don't need
 * to calculate formal HSBA saturation.
 */
p5.ColorConversion._hslaToRGBA = function(hsla){
  var hue = hsla[0] * 6;  // We will split hue into 6 sectors.
  var sat = hsla[1];
  var li = hsla[2];

  var RGBA = [];

  if (sat === 0) {
    RGBA = [li, li, li, hsla[3]]; // Return early if grayscale.
  } else {

    // Calculate brightness.
    var val;
    if (li < 0.5) {
      val = (1 + sat) * li;
    } else {
      val = li + sat - li * sat;
    }

    // Define zest.
    var zest = 2 * li - val;

    // Implement projection (project onto green by default).
    var hzvToRGB = function(hue, zest, val) {
      if (hue < 0) {  // Hue must wrap to allow projection onto red and blue.
        hue += 6;
      } else if (hue >= 6) {
        hue -= 6;
      }
      if (hue < 1) {  // Red to yellow (increasing green).
        return (zest + (val - zest) * hue);
      } else if (hue < 3) {  // Yellow to cyan (greatest green).
        return val;
      } else if (hue < 4) {  // Cyan to blue (decreasing green).
        return (zest + (val - zest) * (4 - hue));
      } else {  // Blue to red (least green).
        return zest;
      }
    };

    // Perform projections, offsetting hue as necessary.
    RGBA = [hzvToRGB(hue + 2, zest, val),
            hzvToRGB(hue    , zest, val),
            hzvToRGB(hue - 2, zest, val),
            hsla[3]];
  }

  return RGBA;
};

/**
 * Convert an RGBA array to HSBA.
 */
p5.ColorConversion._rgbaToHSBA = function(rgba) {
  var red = rgba[0];
  var green = rgba[1];
  var blue = rgba[2];

  var val = Math.max(red, green, blue);
  var chroma = val - Math.min(red, green, blue);

  var hue, sat;
  if (chroma === 0) {  // Return early if grayscale.
    hue = 0;
    sat = 0;
  }
  else {
    sat = chroma / val;
    if (red === val) {  // Magenta to yellow.
      hue = (green - blue) / chroma;
    } else if (green === val) { // Yellow to cyan.
      hue = 2 + (blue - red) / chroma;
    } else if (blue === val) {  // Cyan to magenta.
      hue = 4 + (red - green) / chroma;
    }
    if (hue < 0) {  // Confine hue to the interval [0, 1).
      hue += 6;
    } else if (hue >= 6) {
      hue -= 6;
    }
  }

  return [hue / 6, sat, val, rgba[3]];
};

/**
 * Convert an RGBA array to HSLA.
 */
p5.ColorConversion._rgbaToHSLA = function(rgba) {
  var red = rgba[0];
  var green = rgba[1];
  var blue = rgba[2];

  var val = Math.max(red, green, blue);
  var min = Math.min(red, green, blue);
  var li = val + min;  // We will halve this later.
  var chroma = val - min;

  var hue, sat;
  if (chroma === 0) {  // Return early if grayscale.
    hue = 0;
    sat = 0;
  } else {
    if (li < 1) {
      sat = chroma / li;
    } else {
      sat = chroma / (2 - li);
    }
    if (red === val) {  // Magenta to yellow.
      hue = (green - blue) / chroma;
    } else if (green === val) {  // Yellow to cyan.
      hue = 2 + (blue - red) / chroma;
    } else if (blue === val) {  // Cyan to magenta.
      hue = 4 + (red - green) / chroma;
    }
    if (hue < 0) {  // Confine hue to the interval [0, 1).
      hue += 6;
    } else if (hue >= 6) {
      hue -= 6;
    }
  }

  return [hue / 6, sat, li / 2, rgba[3]];
};

module.exports = p5.ColorConversion;

},{"../core/core":54}],47:[function(_dereq_,module,exports){
/**
 * @module Color
 * @submodule Creating & Reading
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('../core/core');
var constants = _dereq_('../core/constants');
_dereq_('./p5.Color');

/**
 * Extracts the alpha value from a color or pixel array.
 *
 * @method alpha
 * @param {p5.Color|Number[]} obj p5.Color object or pixel array
 * @return {Number} the alpha value
 * @example
 * <div>
 * <code>
 * noStroke();
 * c = color(0, 126, 255, 102);
 * fill(c);
 * rect(15, 15, 35, 70);
 * value = alpha(c);  // Sets 'value' to 102
 * fill(value);
 * rect(50, 15, 35, 70);
 * </code>
 * </div>
 *
 * @alt
 * Left half of canvas light blue and right half light charcoal grey.
 * Left half of canvas light purple and right half a royal blue.
 * Left half of canvas salmon pink and the right half white.
 * Yellow rect in middle right of canvas, with 55 pixel width and height.
 * Yellow ellipse in top left canvas, black ellipse in bottom right,both 80x80.
 * Bright fuschia rect in middle of canvas, 60 pixel width and height.
 * Two bright green rects on opposite sides of the canvas, both 45x80.
 * Four blue rects in each corner of the canvas, each are 35x35.
 * Bright sea green rect on left and darker rect on right of canvas, both 45x80.
 * Dark green rect on left and light green rect on right of canvas, both 45x80.
 * Dark blue rect on left and light teal rect on right of canvas, both 45x80.
 * blue rect on left and green on right, both with black outlines & 35x60.
 * salmon pink rect on left and black on right, both 35x60.
 * 4 rects, tan, brown, brownish purple and purple, with white outlines & 20x60.
 * light pastel green rect on left and dark grey rect on right, both 35x60.
 * yellow rect on left and red rect on right, both with black outlines & 35x60.
 * grey canvas
 * deep pink rect on left and grey rect on right, both 35x60.
 */
p5.prototype.alpha = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getAlpha();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Extracts the blue value from a color or pixel array.
 *
 * @method blue
 * @param {p5.Color|Number[]} obj p5.Color object or pixel array
 * @return {Number} the blue value
 * @example
 * <div>
 * <code>
 * c = color(175, 100, 220);  // Define color 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60);  // Draw left rectangle
 *
 * blueValue = blue(c);  // Get blue in 'c'
 * print(blueValue);  // Prints "220.0"
 * fill(0, 0, blueValue);  // Use 'blueValue' in new fill
 * rect(50, 20, 35, 60);  // Draw right rectangle
 * </code>
 * </div>
 *
 * @alt
 * Left half of canvas light purple and right half a royal blue.
 *
 */
p5.prototype.blue = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getBlue();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Extracts the HSB brightness value from a color or pixel array.
 *
 * @method brightness
 * @param {p5.Color|Number[]} color p5.Color object or pixel array
 * @return {Number} the brightness value
 * @example
 * <div>
 * <code>
 * noStroke();
 * colorMode(HSB, 255);
 * c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * value = brightness(c);  // Sets 'value' to 255
 * fill(value);
 * rect(50, 20, 35, 60);
 * </code>
 * </div>
 *
 * @alt
 * Left half of canvas salmon pink and the right half white.
 *
 */
p5.prototype.brightness = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getBrightness();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Creates colors for storing in variables of the color datatype. The
 * parameters are interpreted as RGB or HSB values depending on the
 * current colorMode(). The default mode is RGB values from 0 to 255
 * and, therefore, the function call color(255, 204, 0) will return a
 * bright yellow color.
 * <br><br>
 * Note that if only one value is provided to color(), it will be interpreted
 * as a grayscale value. Add a second value, and it will be used for alpha
 * transparency. When three values are specified, they are interpreted as
 * either RGB or HSB values. Adding a fourth value applies alpha
 * transparency.
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color
 * strings and all named color strings are supported. In this case, an alpha
 * number value as a second argument is not supported, the RGBA form should be
 * used.
 *
 * @method color
 * @param  {Number}        gray    number specifying value between white
 *                                 and black.
 * @param  {Number}        [alpha] alpha value relative to current color range
 *                                 (default is 0-255)
 * @return {p5.Color}              resulting color
 *
 * @example
 * <div>
 * <code>
 * var c = color(255, 204, 0);  // Define color 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * noStroke();  // Don't draw a stroke around shapes
 * rect(30, 20, 55, 55);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var c = color(255, 204, 0);  // Define color 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * noStroke();  // Don't draw a stroke around shapes
 * ellipse(25, 25, 80, 80);  // Draw left circle
 *
 * // Using only one value with color()
 * // generates a grayscale value.
 * var c = color(65);  // Update 'c' with grayscale value
 * fill(c);  // Use updated 'c' as fill color
 * ellipse(75, 75, 80, 80);  // Draw right circle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // Named SVG & CSS colors may be used,
 * var c = color('magenta');
 * fill(c);  // Use 'c' as fill color
 * noStroke();  // Don't draw a stroke around shapes
 * rect(20, 20, 60, 60);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // as can hex color codes:
 * noStroke();  // Don't draw a stroke around shapes
 * var c = color('#0f0');
 * fill(c);  // Use 'c' as fill color
 * rect(0, 10, 45, 80);  // Draw rectangle
 *
 * c = color('#00ff00');
 * fill(c);  // Use updated 'c' as fill color
 * rect(55, 10, 45, 80);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // RGB and RGBA color strings are also supported:
 * // these all set to the same color (solid blue)
 * var c;
 * noStroke();  // Don't draw a stroke around shapes
 * c = color('rgb(0,0,255)');
 * fill(c); // Use 'c' as fill color
 * rect(10, 10, 35, 35);  // Draw rectangle
 *
 * c = color('rgb(0%, 0%, 100%)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 35, 35);  // Draw rectangle
 *
 * c = color('rgba(0, 0, 255, 1)');
 * fill(c); // Use updated 'c' as fill color
 * rect(10, 55, 35, 35);  // Draw rectangle
 *
 * c = color('rgba(0%, 0%, 100%, 1)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 55, 35, 35);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // HSL color is also supported and can be specified
 * // by value
 * var c;
 * noStroke();  // Don't draw a stroke around shapes
 * c = color('hsl(160, 100%, 50%)');
 * fill(c);  // Use 'c' as fill color
 * rect(0, 10, 45, 80);  // Draw rectangle
 *
 * c = color('hsla(160, 100%, 50%, 0.5)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // HSB color is also supported and can be specified
 * // by value
 * var c;
 * noStroke();  // Don't draw a stroke around shapes
 * c = color('hsb(160, 100%, 50%)');
 * fill(c);  // Use 'c' as fill color
 * rect(0, 10, 45, 80);  // Draw rectangle
 *
 * c = color('hsba(160, 100%, 50%, 0.5)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80);  // Draw rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var c;  // Declare color 'c'
 * noStroke();  // Don't draw a stroke around shapes
 *
 * // If no colorMode is specified, then the
 * // default of RGB with scale of 0-255 is used.
 * c = color(50, 55, 100);  // Create a color for 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * rect(0, 10, 45, 80);  // Draw left rect
 *
 * colorMode(HSB, 100);  // Use HSB with scale of 0-100
 * c = color(50, 55, 100);  // Update 'c' with new color
 * fill(c);  // Use updated 'c' as fill color
 * rect(55, 10, 45, 80);  // Draw right rect
 * </code>
 * </div>
 *
 * @alt
 * Yellow rect in middle right of canvas, with 55 pixel width and height.
 * Yellow ellipse in top left of canvas, black ellipse in bottom right,both 80x80.
 * Bright fuschia rect in middle of canvas, 60 pixel width and height.
 * Two bright green rects on opposite sides of the canvas, both 45x80.
 * Four blue rects in each corner of the canvas, each are 35x35.
 * Bright sea green rect on left and darker rect on right of canvas, both 45x80.
 * Dark green rect on left and lighter green rect on right of canvas, both 45x80.
 * Dark blue rect on left and light teal rect on right of canvas, both 45x80.
 *
 */

/**
 * @method color
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @return {p5.Color}
 */

/**
 * @method color
 * @param  {String}        value   a color string
 * @param  {Number}        [alpha]
 * @return {p5.Color}
 */

/**
 * @method color
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 * @return {p5.Color}
 */

p5.prototype.color = function() {
  if (arguments[0] instanceof p5.Color) {
    return arguments[0];  // Do nothing if argument is already a color object.
  } else if (arguments[0] instanceof Array) {
    if (this instanceof p5.Renderer) {
      return new p5.Color(this, arguments[0]);
    } else {
      return new p5.Color(this._renderer, arguments[0]);
    }
  } else {
    if (this instanceof p5.Renderer) {
      return new p5.Color(this, arguments);
    } else {
      return new p5.Color(this._renderer, arguments);
    }
  }
};

/**
 * Extracts the green value from a color or pixel array.
 *
 * @method green
 * @param {p5.Color|Number[]} color p5.Color object or pixel array
 * @return {Number} the green value
 * @example
 * <div>
 * <code>
 * c = color(20, 75, 200);  // Define color 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60);  // Draw left rectangle
 *
 * greenValue = green(c);  // Get green in 'c'
 * print(greenValue);  // Print "75.0"
 * fill(0, greenValue, 0);  // Use 'greenValue' in new fill
 * rect(50, 20, 35, 60);  // Draw right rectangle
 * </code>
 * </div>
 *
 * @alt
 * blue rect on left and green on right, both with black outlines & 35x60.
 *
 */

p5.prototype.green = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getGreen();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Extracts the hue value from a color or pixel array.
 *
 * Hue exists in both HSB and HSL. This function will return the
 * HSB-normalized hue when supplied with an HSB color object (or when supplied
 * with a pixel array while the color mode is HSB), but will default to the
 * HSL-normalized hue otherwise. (The values will only be different if the
 * maximum hue setting for each system is different.)
 *
 * @method hue
 * @param {p5.Color|Number[]} color p5.Color object or pixel array
 * @return {Number} the hue
 * @example
 * <div>
 * <code>
 * noStroke();
 * colorMode(HSB, 255);
 * c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * value = hue(c);  // Sets 'value' to "0"
 * fill(value);
 * rect(50, 20, 35, 60);
 * </code>
 * </div>
 *
 * @alt
 * salmon pink rect on left and black on right, both 35x60.
 *
 */

p5.prototype.hue = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getHue();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Blends two colors to find a third color somewhere between them. The amt
 * parameter is the amount to interpolate between the two values where 0.0
 * equal to the first color, 0.1 is very near the first color, 0.5 is halfway
 * in between, etc. An amount below 0 will be treated as 0. Likewise, amounts
 * above 1 will be capped at 1. This is different from the behavior of lerp(),
 * but necessary because otherwise numbers outside the range will produce
 * strange and unexpected colors.
 * <br><br>
 * The way that colours are interpolated depends on the current color mode.
 *
 * @method lerpColor
 * @param  {p5.Color} c1  interpolate from this color
 * @param  {p5.Color} c2  interpolate to this color
 * @param  {Number}       amt number between 0 and 1
 * @return {p5.Color}     interpolated color
 * @example
 * <div>
 * <code>
 * colorMode(RGB);
 * stroke(255);
 * background(51);
 * from = color(218, 165, 32);
 * to = color(72, 61, 139);
 * colorMode(RGB);  // Try changing to HSB.
 * interA = lerpColor(from, to, .33);
 * interB = lerpColor(from, to, .66);
 * fill(from);
 * rect(10, 20, 20, 60);
 * fill(interA);
 * rect(30, 20, 20, 60);
 * fill(interB);
 * rect(50, 20, 20, 60);
 * fill(to);
 * rect(70, 20, 20, 60);
 * </code>
 * </div>
 *
 * @alt
 * 4 rects one tan, brown, brownish purple, purple, with white outlines & 20x60
 *
 */

p5.prototype.lerpColor = function(c1, c2, amt) {
  var mode = this._renderer._colorMode;
  var maxes = this._renderer._colorMaxes;
  var l0, l1, l2, l3;
  var fromArray, toArray;

  if (mode === constants.RGB) {
    fromArray = c1.levels.map(function(level) {
      return level / 255;
    });
    toArray = c2.levels.map(function(level) {
      return level / 255;
    });
  } else if (mode === constants.HSB) {
    c1._getBrightness();  // Cache hsba so it definitely exists.
    c2._getBrightness();
    fromArray = c1.hsba;
    toArray = c2.hsba;
  } else if (mode === constants.HSL) {
    c1._getLightness();  // Cache hsla so it definitely exists.
    c2._getLightness();
    fromArray = c1.hsla;
    toArray = c2.hsla;
  } else {
    throw new Error (mode + 'cannot be used for interpolation.');
  }

  // Prevent extrapolation.
  amt = Math.max(Math.min(amt, 1), 0);

  // Define lerp here itself if user isn't using math module.
  // Maintains the definition as found in math/calculation.js
  if(typeof this.lerp === 'undefined') {
    this.lerp = function (start, stop, amt) {
      return amt*(stop-start)+start;
    };
  }

  // Perform interpolation.
  l0 = this.lerp(fromArray[0], toArray[0], amt);
  l1 = this.lerp(fromArray[1], toArray[1], amt);
  l2 = this.lerp(fromArray[2], toArray[2], amt);
  l3 = this.lerp(fromArray[3], toArray[3], amt);

  // Scale components.
  l0 *= maxes[mode][0];
  l1 *= maxes[mode][1];
  l2 *= maxes[mode][2];
  l3 *= maxes[mode][3];

  return this.color(l0, l1, l2, l3);
};

/**
 * Extracts the HSL lightness value from a color or pixel array.
 *
 * @method lightness
 * @param {p5.Color|Number[]} color p5.Color object or pixel array
 * @return {Number} the lightness
 * @example
 * <div>
 * <code>
 * noStroke();
 * colorMode(HSL);
 * c = color(156, 100, 50, 1);
 * fill(c);
 * rect(15, 20, 35, 60);
 * value = lightness(c);  // Sets 'value' to 50
 * fill(value);
 * rect(50, 20, 35, 60);
 * </code>
 * </div>
 *
 * @alt
 * light pastel green rect on left and dark grey rect on right, both 35x60.
 *
 */
p5.prototype.lightness = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getLightness();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Extracts the red value from a color or pixel array.
 *
 * @method red
 * @param {p5.Color|Number[]} obj p5.Color object or pixel array
 * @return {Number} the red value
 * @example
 * <div>
 * <code>
 * c = color(255, 204, 0);  // Define color 'c'
 * fill(c);  // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60);  // Draw left rectangle
 *
 * redValue = red(c);  // Get red in 'c'
 * print(redValue);  // Print "255.0"
 * fill(redValue, 0, 0);  // Use 'redValue' in new fill
 * rect(50, 20, 35, 60);  // Draw right rectangle
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * colorMode(RGB, 255);
 * var c = color(127, 255, 0);
 * colorMode(RGB, 1);
 * var myColor = red(c);
 * print(myColor);
 * </code>
 * </div>
 *
 * @alt
 * yellow rect on left and red rect on right, both with black outlines and 35x60.
 * grey canvas
 */
p5.prototype.red = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getRed();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

/**
 * Extracts the saturation value from a color or pixel array.
 *
 * Saturation is scaled differently in HSB and HSL. This function will return
 * the HSB saturation when supplied with an HSB color object (or when supplied
 * with a pixel array while the color mode is HSB), but will default to the
 * HSL saturation otherwise.
 *
 * @method saturation
 * @param {p5.Color|Number[]} color p5.Color object or pixel array
 * @return {Number} the saturation
 * @example
 * <div>
 * <code>
 * noStroke();
 * colorMode(HSB, 255);
 * c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * value = saturation(c);  // Sets 'value' to 126
 * fill(value);
 * rect(50, 20, 35, 60);
 * </code>
 * </div>
 *
 * @alt
 *deep pink rect on left and grey rect on right, both 35x60.
 *
 */

p5.prototype.saturation = function(c) {
  if (c instanceof p5.Color || c instanceof Array) {
    return this.color(c)._getSaturation();
  } else {
    throw new Error('Needs p5.Color or pixel array as argument.');
  }
};

module.exports = p5;

},{"../core/constants":53,"../core/core":54,"./p5.Color":48}],48:[function(_dereq_,module,exports){
/**
 * @module Color
 * @submodule Creating & Reading
 * @for p5
 * @requires core
 * @requires constants
 * @requires color_conversion
 */

var p5 = _dereq_('../core/core');
var constants = _dereq_('../core/constants');
var color_conversion = _dereq_('./color_conversion');

/**
 * We define colors to be immutable objects. Each color stores the color mode
 * and level maxes that applied at the time of its construction. These are
 * used to interpret the input arguments and to format the output e.g. when
 * saturation() is requested.
 *
 * Internally we store an array representing the ideal RGBA values in floating
 * point form, normalized from 0 to 1. From this we calculate the closest
 * screen color (RGBA levels from 0 to 255) and expose this to the renderer.
 *
 * We also cache normalized, floating point components of the color in various
 * representations as they are calculated. This is done to prevent repeating a
 * conversion that has already been performed.
 *
 * @class p5.Color
 * @constructor
 */
p5.Color = function(renderer, vals) {

  // Record color mode and maxes at time of construction.
  this.mode = renderer._colorMode;
  this.maxes = renderer._colorMaxes;

  // Calculate normalized RGBA values.
  if (this.mode !== constants.RGB &&
      this.mode !== constants.HSL &&
      this.mode !== constants.HSB) {
    throw new Error(this.mode + ' is an invalid colorMode.');
  } else {
    this._array = p5.Color._parseInputs.apply(renderer, vals);
  }

  // Expose closest screen color.
  this.levels = this._array.map(function(level) {
    return Math.round(level * 255);
  });

  return this;
};

p5.Color.prototype.toString = function() {
  var a = this.levels;
  var alpha = this._array[3];  // String representation uses normalized alpha.
  return 'rgba('+a[0]+','+a[1]+','+a[2]+','+ alpha +')';
};

p5.Color.prototype._getAlpha = function() {
  return this._array[3] * this.maxes[this.mode][3];
};

p5.Color.prototype._getBlue = function() {
  return this._array[2] * this.maxes[constants.RGB][2];
};

p5.Color.prototype._getBrightness = function() {
  if (!this.hsba) {
    this.hsba = color_conversion._rgbaToHSBA(this._array);
  }
  return this.hsba[2] * this.maxes[constants.HSB][2];
};

p5.Color.prototype._getGreen = function() {
  return this._array[1] * this.maxes[constants.RGB][1];
};

/**
 * Hue is the same in HSB and HSL, but the maximum value may be different.
 * This function will return the HSB-normalized saturation when supplied with
 * an HSB color object, but will default to the HSL-normalized saturation
 * otherwise.
 */
p5.Color.prototype._getHue = function() {
  if (this.mode === constants.HSB) {
    if (!this.hsba) {
      this.hsba = color_conversion._rgbaToHSBA(this._array);
    }
    return this.hsba[0] * this.maxes[constants.HSB][0];
  } else {
    if (!this.hsla) {
      this.hsla = color_conversion._rgbaToHSLA(this._array);
    }
    return this.hsla[0] * this.maxes[constants.HSL][0];
  }
};

p5.Color.prototype._getLightness = function() {
  if (!this.hsla) {
    this.hsla = color_conversion._rgbaToHSLA(this._array);
  }
  return this.hsla[2] * this.maxes[constants.HSL][2];
};

p5.Color.prototype._getRed = function() {
  return this._array[0] * this.maxes[constants.RGB][0];
};

/**
 * Saturation is scaled differently in HSB and HSL. This function will return
 * the HSB saturation when supplied with an HSB color object, but will default
 * to the HSL saturation otherwise.
 */
p5.Color.prototype._getSaturation = function() {
  if (this.mode === constants.HSB) {
    if (!this.hsba) {
      this.hsba = color_conversion._rgbaToHSBA(this._array);
    }
    return this.hsba[1] * this.maxes[constants.HSB][1];
  } else {
    if (!this.hsla) {
      this.hsla = color_conversion._rgbaToHSLA(this._array);
    }
    return this.hsla[1] * this.maxes[constants.HSL][1];
  }
};

/**
 * CSS named colors.
 */
var namedColors = {
  aliceblue:             '#f0f8ff',
  antiquewhite:          '#faebd7',
  aqua:                  '#00ffff',
  aquamarine:            '#7fffd4',
  azure:                 '#f0ffff',
  beige:                 '#f5f5dc',
  bisque:                '#ffe4c4',
  black:                 '#000000',
  blanchedalmond:        '#ffebcd',
  blue:                  '#0000ff',
  blueviolet:            '#8a2be2',
  brown:                 '#a52a2a',
  burlywood:             '#deb887',
  cadetblue:             '#5f9ea0',
  chartreuse:            '#7fff00',
  chocolate:             '#d2691e',
  coral:                 '#ff7f50',
  cornflowerblue:        '#6495ed',
  cornsilk:              '#fff8dc',
  crimson:               '#dc143c',
  cyan:                  '#00ffff',
  darkblue:              '#00008b',
  darkcyan:              '#008b8b',
  darkgoldenrod:         '#b8860b',
  darkgray:              '#a9a9a9',
  darkgreen:             '#006400',
  darkgrey:              '#a9a9a9',
  darkkhaki:             '#bdb76b',
  darkmagenta:           '#8b008b',
  darkolivegreen:        '#556b2f',
  darkorange:            '#ff8c00',
  darkorchid:            '#9932cc',
  darkred:               '#8b0000',
  darksalmon:            '#e9967a',
  darkseagreen:          '#8fbc8f',
  darkslateblue:         '#483d8b',
  darkslategray:         '#2f4f4f',
  darkslategrey:         '#2f4f4f',
  darkturquoise:         '#00ced1',
  darkviolet:            '#9400d3',
  deeppink:              '#ff1493',
  deepskyblue:           '#00bfff',
  dimgray:               '#696969',
  dimgrey:               '#696969',
  dodgerblue:            '#1e90ff',
  firebrick:             '#b22222',
  floralwhite:           '#fffaf0',
  forestgreen:           '#228b22',
  fuchsia:               '#ff00ff',
  gainsboro:             '#dcdcdc',
  ghostwhite:            '#f8f8ff',
  gold:                  '#ffd700',
  goldenrod:             '#daa520',
  gray:                  '#808080',
  green:                 '#008000',
  greenyellow:           '#adff2f',
  grey:                  '#808080',
  honeydew:              '#f0fff0',
  hotpink:               '#ff69b4',
  indianred:             '#cd5c5c',
  indigo:                '#4b0082',
  ivory:                 '#fffff0',
  khaki:                 '#f0e68c',
  lavender:              '#e6e6fa',
  lavenderblush:         '#fff0f5',
  lawngreen:             '#7cfc00',
  lemonchiffon:          '#fffacd',
  lightblue:             '#add8e6',
  lightcoral:            '#f08080',
  lightcyan:             '#e0ffff',
  lightgoldenrodyellow:  '#fafad2',
  lightgray:             '#d3d3d3',
  lightgreen:            '#90ee90',
  lightgrey:             '#d3d3d3',
  lightpink:             '#ffb6c1',
  lightsalmon:           '#ffa07a',
  lightseagreen:         '#20b2aa',
  lightskyblue:          '#87cefa',
  lightslategray:        '#778899',
  lightslategrey:        '#778899',
  lightsteelblue:        '#b0c4de',
  lightyellow:           '#ffffe0',
  lime:                  '#00ff00',
  limegreen:             '#32cd32',
  linen:                 '#faf0e6',
  magenta:               '#ff00ff',
  maroon:                '#800000',
  mediumaquamarine:      '#66cdaa',
  mediumblue:            '#0000cd',
  mediumorchid:          '#ba55d3',
  mediumpurple:          '#9370db',
  mediumseagreen:        '#3cb371',
  mediumslateblue:       '#7b68ee',
  mediumspringgreen:     '#00fa9a',
  mediumturquoise:       '#48d1cc',
  mediumvioletred:       '#c71585',
  midnightblue:          '#191970',
  mintcream:             '#f5fffa',
  mistyrose:             '#ffe4e1',
  moccasin:              '#ffe4b5',
  navajowhite:           '#ffdead',
  navy:                  '#000080',
  oldlace:               '#fdf5e6',
  olive:                 '#808000',
  olivedrab:             '#6b8e23',
  orange:                '#ffa500',
  orangered:             '#ff4500',
  orchid:                '#da70d6',
  palegoldenrod:         '#eee8aa',
  palegreen:             '#98fb98',
  paleturquoise:         '#afeeee',
  palevioletred:         '#db7093',
  papayawhip:            '#ffefd5',
  peachpuff:             '#ffdab9',
  peru:                  '#cd853f',
  pink:                  '#ffc0cb',
  plum:                  '#dda0dd',
  powderblue:            '#b0e0e6',
  purple:                '#800080',
  red:                   '#ff0000',
  rosybrown:             '#bc8f8f',
  royalblue:             '#4169e1',
  saddlebrown:           '#8b4513',
  salmon:                '#fa8072',
  sandybrown:            '#f4a460',
  seagreen:              '#2e8b57',
  seashell:              '#fff5ee',
  sienna:                '#a0522d',
  silver:                '#c0c0c0',
  skyblue:               '#87ceeb',
  slateblue:             '#6a5acd',
  slategray:             '#708090',
  slategrey:             '#708090',
  snow:                  '#fffafa',
  springgreen:           '#00ff7f',
  steelblue:             '#4682b4',
  tan:                   '#d2b48c',
  teal:                  '#008080',
  thistle:               '#d8bfd8',
  tomato:                '#ff6347',
  turquoise:             '#40e0d0',
  violet:                '#ee82ee',
  wheat:                 '#f5deb3',
  white:                 '#ffffff',
  whitesmoke:            '#f5f5f5',
  yellow:                '#ffff00',
  yellowgreen:           '#9acd32'
};

/**
 * These regular expressions are used to build up the patterns for matching
 * viable CSS color strings: fragmenting the regexes in this way increases the
 * legibility and comprehensibility of the code.
 *
 * Note that RGB values of .9 are not parsed by IE, but are supported here for
 * color string consistency.
 */
var WHITESPACE = /\s*/;  // Match zero or more whitespace characters.
var INTEGER = /(\d{1,3})/;  // Match integers: 79, 255, etc.
var DECIMAL = /((?:\d+(?:\.\d+)?)|(?:\.\d+))/;  // Match 129.6, 79, .9, etc.
var PERCENT = new RegExp(DECIMAL.source + '%');  // Match 12.9%, 79%, .9%, etc.

/**
 * Full color string patterns. The capture groups are necessary.
 */
var colorPatterns = {
  // Match colors in format #XXX, e.g. #416.
  HEX3: /^#([a-f0-9])([a-f0-9])([a-f0-9])$/i,

  // Match colors in format #XXXX, e.g. #5123.
  HEX4: /^#([a-f0-9])([a-f0-9])([a-f0-9])([a-f0-9])$/i,

  // Match colors in format #XXXXXX, e.g. #b4d455.
  HEX6: /^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$/i,

  // Match colors in format #XXXXXXXX, e.g. #b4d45535.
  HEX8: /^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$/i,

  // Match colors in format rgb(R, G, B), e.g. rgb(255, 0, 128).
  RGB: new RegExp([
    '^rgb\\(',
    INTEGER.source,
    ',',
    INTEGER.source,
    ',',
    INTEGER.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format rgb(R%, G%, B%), e.g. rgb(100%, 0%, 28.9%).
  RGB_PERCENT: new RegExp([
    '^rgb\\(',
    PERCENT.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format rgb(R, G, B, A), e.g. rgb(255, 0, 128, 0.25).
  RGBA: new RegExp([
    '^rgba\\(',
    INTEGER.source,
    ',',
    INTEGER.source,
    ',',
    INTEGER.source,
    ',',
    DECIMAL.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format rgb(R%, G%, B%, A), e.g. rgb(100%, 0%, 28.9%, 0.5).
  RGBA_PERCENT: new RegExp([
    '^rgba\\(',
    PERCENT.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    ',',
    DECIMAL.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format hsla(H, S%, L%), e.g. hsl(100, 40%, 28.9%).
  HSL: new RegExp([
    '^hsl\\(',
    INTEGER.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format hsla(H, S%, L%, A), e.g. hsla(100, 40%, 28.9%, 0.5).
  HSLA: new RegExp([
    '^hsla\\(',
    INTEGER.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    ',',
    DECIMAL.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format hsb(H, S%, B%), e.g. hsb(100, 40%, 28.9%).
  HSB: new RegExp([
    '^hsb\\(',
    INTEGER.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i'),

  // Match colors in format hsba(H, S%, B%, A), e.g. hsba(100, 40%, 28.9%, 0.5).
  HSBA: new RegExp([
    '^hsba\\(',
    INTEGER.source,
    ',',
    PERCENT.source,
    ',',
    PERCENT.source,
    ',',
    DECIMAL.source,
    '\\)$'
  ].join(WHITESPACE.source), 'i')
};

/**
 * For a number of different inputs, returns a color formatted as [r, g, b, a]
 * arrays, with each component normalized between 0 and 1.
 *
 * @param {Array} [...args] An 'array-like' object that represents a list of
 *                          arguments
 * @return {Number[]}       a color formatted as [r, g, b, a]
 *                          Example:
 *                          input        ==> output
 *                          g            ==> [g, g, g, 255]
 *                          g,a          ==> [g, g, g, a]
 *                          r, g, b      ==> [r, g, b, 255]
 *                          r, g, b, a   ==> [r, g, b, a]
 *                          [g]          ==> [g, g, g, 255]
 *                          [g, a]       ==> [g, g, g, a]
 *                          [r, g, b]    ==> [r, g, b, 255]
 *                          [r, g, b, a] ==> [r, g, b, a]
 * @example
 * <div>
 * <code>
 * // todo
 * </code>
 * </div>
 *
 * @alt
 * //todo
 *
 */
p5.Color._parseInputs = function() {
  var numArgs = arguments.length;
  var mode = this._colorMode;
  var maxes = this._colorMaxes;
  var results = [];

  if (numArgs >= 3) {  // Argument is a list of component values.

    results[0] = arguments[0] / maxes[mode][0];
    results[1] = arguments[1] / maxes[mode][1];
    results[2] = arguments[2] / maxes[mode][2];

    // Alpha may be undefined, so default it to 100%.
    if (typeof arguments[3] === 'number') {
      results[3] = arguments[3] / maxes[mode][3];
    } else {
      results[3] = 1;
    }

    // Constrain components to the range [0,1].
    results = results.map(function(value) {
      return Math.max(Math.min(value, 1), 0);
    });

    // Convert to RGBA and return.
    if (mode === constants.HSL) {
      return color_conversion._hslaToRGBA(results);
    } else if (mode === constants.HSB) {
      return color_conversion._hsbaToRGBA(results);
    } else {
      return results;
    }

  } else if (numArgs === 1 && typeof arguments[0] === 'string') {

    var str = arguments[0].trim().toLowerCase();

    // Return if string is a named colour.
    if (namedColors[str]) {
      return p5.Color._parseInputs.apply(this, [namedColors[str]]);
    }

    // Try RGBA pattern matching.
    if (colorPatterns.HEX3.test(str)) {  // #rgb
      results = colorPatterns.HEX3.exec(str).slice(1).map(function(color) {
        return parseInt(color + color, 16) / 255;
      });
      results[3] = 1;
      return results;
    } else if (colorPatterns.HEX6.test(str)) {  // #rrggbb
      results = colorPatterns.HEX6.exec(str).slice(1).map(function(color) {
        return parseInt(color, 16) / 255;
      });
      results[3] = 1;
      return results;
    } else if (colorPatterns.HEX4.test(str)) {  // #rgba
      results = colorPatterns.HEX4.exec(str).slice(1).map(function(color) {
        return parseInt(color + color, 16) / 255;
      });
      return results;
    } else if (colorPatterns.HEX8.test(str)) {  // #rrggbbaa
      results = colorPatterns.HEX8.exec(str).slice(1).map(function(color) {
        return parseInt(color, 16) / 255;
      });
      return results;
    } else if (colorPatterns.RGB.test(str)) {  // rgb(R,G,B)
      results = colorPatterns.RGB.exec(str).slice(1).map(function(color) {
        return color / 255;
      });
      results[3] = 1;
      return results;
    } else if (colorPatterns.RGB_PERCENT.test(str)) {  // rgb(R%,G%,B%)
      results = colorPatterns.RGB_PERCENT.exec(str).slice(1)
        .map(function(color) {
          return parseFloat(color) / 100;
        });
      results[3] = 1;
      return results;
    } else if (colorPatterns.RGBA.test(str)) {  // rgba(R,G,B,A)
      results = colorPatterns.RGBA.exec(str).slice(1)
        .map(function(color, idx) {
          if (idx === 3) {
            return parseFloat(color);
          }
          return color / 255;
        });
      return results;
    } else if (colorPatterns.RGBA_PERCENT.test(str)) {  // rgba(R%,G%,B%,A%)
      results = colorPatterns.RGBA_PERCENT.exec(str).slice(1)
        .map(function(color, idx) {
          if (idx === 3) {
            return parseFloat(color);
          }
          return parseFloat(color) / 100;
        });
      return results;
    }

    // Try HSLA pattern matching.
    if (colorPatterns.HSL.test(str)) {  // hsl(H,S,L)
      results = colorPatterns.HSL.exec(str).slice(1)
        .map(function(color, idx) {
        if (idx === 0) {
          return parseInt(color, 10) / 360;
        }
        return parseInt(color, 10) / 100;
      });
      results[3] = 1;
    } else if (colorPatterns.HSLA.test(str)) {  // hsla(H,S,L,A)
      results = colorPatterns.HSLA.exec(str).slice(1)
        .map(function(color, idx) {
        if (idx === 0) {
          return parseInt(color, 10) / 360;
        }
        else if (idx === 3) {
          return parseFloat(color);
        }
        return parseInt(color, 10) / 100;
      });
    }
    results = results.map(function(value) {
      return Math.max(Math.min(value, 1), 0);
    });
    if (results.length) {
      return color_conversion._hslaToRGBA(results);
    }

    // Try HSBA pattern matching.
    if (colorPatterns.HSB.test(str)) {  // hsb(H,S,B)
      results = colorPatterns.HSB.exec(str).slice(1)
        .map(function(color, idx) {
        if (idx === 0) {
          return parseInt(color, 10) / 360;
        }
        return parseInt(color, 10) / 100;
      });
      results[3] = 1;
    } else if (colorPatterns.HSBA.test(str)) {  // hsba(H,S,B,A)
      results = colorPatterns.HSBA.exec(str).slice(1)
        .map(function(color, idx) {
        if (idx === 0) {
          return parseInt(color, 10) / 360;
        }
        else if (idx === 3) {
          return parseFloat(color);
        }
        return parseInt(color, 10) / 100;
      });
    }
    results = results.map(function(value) {
      return Math.max(Math.min(value, 1), 0);
    });

    if (results.length) {
      return color_conversion._hsbaToRGBA(results);
    }

    // Input did not match any CSS color pattern: default to white.
    results = [1, 1, 1, 1];

  } else if ((numArgs === 1 || numArgs === 2) &&
              typeof arguments[0] === 'number') {  // 'Grayscale' mode.

    /**
     * For HSB and HSL, interpret the gray level as a brightness/lightness
     * value (they are equivalent when chroma is zero). For RGB, normalize the
     * gray level according to the blue maximum.
     */
    results[0] = arguments[0] / maxes[mode][2];
    results[1] = arguments[0] / maxes[mode][2];
    results[2] = arguments[0] / maxes[mode][2];

    // Alpha may be undefined, so default it to 100%.
    if (typeof arguments[1] === 'number') {
      results[3] = arguments[1] / maxes[mode][3];
    } else {
      results[3] = 1;
    }

    // Constrain components to the range [0,1].
    results = results.map(function(value) {
      return Math.max(Math.min(value, 1), 0);
    });

  } else {
    throw new Error (arguments + 'is not a valid color representation.');
  }

  return results;
};

module.exports = p5.Color;

},{"../core/constants":53,"../core/core":54,"./color_conversion":46}],49:[function(_dereq_,module,exports){
/**
 * @module Color
 * @submodule Setting
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('../core/core');
var constants = _dereq_('../core/constants');
_dereq_('./p5.Color');

/**
 * The background() function sets the color used for the background of the
 * p5.js canvas. The default background is light gray. This function is
 * typically used within draw() to clear the display window at the beginning
 * of each frame, but it can be used inside setup() to set the background on
 * the first frame of animation or if the background need only be set once.
 * <br><br>
 * The color is either specified in terms of the RGB, HSB, or HSL color
 * depending on the current colorMode. (The default color space is RGB, with
 * each value in the range from 0 to 255).
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color strings
 * and all named color strings are supported. In this case, an alpha number
 * value as a second argument is not supported, the RGBA form should be used.
 * <br><br>
 * A p5.Color object can also be provided to set the background color.
 * <br><br>
 * A p5.Image can also be provided to set the background iamge.
 *
 * @method background
 * @param {p5.Color} color     any value created by the color() function
 * @param {Number} [a]         opacity of the background relative to current
 *                             color range (default is 0-100)
 * @chainable
 *
 * @example
 * <div>
 * <code>
 * // Grayscale integer value
 * background(51);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // R, G & B integer values
 * background(255, 204, 0);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // H, S & B integer values
 * colorMode(HSB);
 * background(255, 204, 100);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // Named SVG/CSS color string
 * background('red');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // three-digit hexadecimal RGB notation
 * background('#fae');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // six-digit hexadecimal RGB notation
 * background('#222222');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGB notation
 * background('rgb(0,255,0)');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGBA notation
 * background('rgba(0,255,0, 0.25)');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGB notation
 * background('rgb(100%,0%,10%)');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGBA notation
 * background('rgba(100%,0%,100%,0.5)');
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // p5 Color object
 * background(color(0, 0, 255));
 * </code>
 * </div>
 *
 * @alt
 * canvas with darkest charcoal grey background.
 * canvas with yellow background.
 * canvas with royal blue background.
 * canvas with red background.
 * canvas with pink background.
 * canvas with black background.
 * canvas with bright green background.
 * canvas with soft green background.
 * canvas with red background.
 * canvas with light purple background.
 * canvas with blue background.
 */

/**
 * @method background
 * @param {String} colorstring color string, possible formats include: integer
 *                         rgb() or rgba(), percentage rgb() or rgba(),
 *                         3-digit hex, 6-digit hex
 * @param {Number} [a]
 * @chainable
 */

/**
 * @method background
 * @param {Number} gray   specifies a value between white and black
 * @param {Number} [a]
 * @chainable
 */

/**
 * @method background
 * @param {Number} v1     red or hue value (depending on the current color
 *                        mode)
 * @param {Number} v2     green or saturation value (depending on the current
 *                        color mode)
 * @param {Number} v3     blue or brightness value (depending on the current
 *                        color mode)
 * @param  {Number} [a]
 * @chainable
 */

/**
 * @method background
 * @param {p5.Image} image     image created with loadImage() or createImage(),
 *                             to set as background
 *                             (must be same size as the sketch window)
 * @param  {Number}  [a]
 * @chainable
 */
p5.prototype.background = function() {
  if (arguments[0] instanceof p5.Image) {
    this.image(arguments[0], 0, 0, this.width, this.height);
  } else {
    this._renderer.background.apply(this._renderer, arguments);
  }
  return this;
};

/**
 * Clears the pixels within a buffer. This function only works on p5.Canvas
 * objects created with the createCanvas() function; it won't work with the
 * main display window. Unlike the main graphics context, pixels in
 * additional graphics areas created with createGraphics() can be entirely
 * or partially transparent. This function clears everything to make all of
 * the pixels 100% transparent.
 *
 * @method clear
 * @chainable
 * @example
 * <div>
 * <code>
 * // Clear the screen on mouse press.
 * function setup() {
 *   createCanvas(100, 100);
 * }
 *
 * function draw() {
 *   ellipse(mouseX, mouseY, 20, 20);
 * }
 *
 * function mousePressed() {
 *   clear();
 * }
 * </code>
 * </div>
 *
 * @alt
 * 20x20 white ellipses are continually drawn at mouse x and y coordinates.
 *
 */

p5.prototype.clear = function() {
  this._renderer.clear();
  return this;
};

/**
 * colorMode() changes the way p5.js interprets color data. By default, the
 * parameters for fill(), stroke(), background(), and color() are defined by
 * values between 0 and 255 using the RGB color model. This is equivalent to
 * setting colorMode(RGB, 255). Setting colorMode(HSB) lets you use the HSB
 * system instead. By default, this is colorMode(HSB, 360, 100, 100, 1). You
 * can also use HSL.
 * <br><br>
 * Note: existing color objects remember the mode that they were created in,
 * so you can change modes as you like without affecting their appearance.
 *
 *
 * @method colorMode
 * @param {Constant} mode   either RGB, HSB or HSL, corresponding to
 *                          Red/Green/Blue and Hue/Saturation/Brightness
 *                          (or Lightness)
 * @param {Number}  [max]  range for all values
 * @chainable
 */
/**
 * @method colorMode
 * @param {Constant} mode
 * @param {Number} max1     range for the red or hue depending on the
 *                              current color mode
 * @param {Number} max2     range for the green or saturation depending
 *                              on the current color mode
 * @param {Number} max3     range for the blue or brightness/lighntess
 *                              depending on the current color mode
 * @param {Number} [maxA]   range for the alpha
 * @chainable
 *
 * @example
 * <div>
 * <code>
 * noStroke();
 * colorMode(RGB, 100);
 * for (i = 0; i < 100; i++) {
 *   for (j = 0; j < 100; j++) {
 *     stroke(i, j, 0);
 *     point(i, j);
 *   }
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noStroke();
 * colorMode(HSB, 100);
 * for (i = 0; i < 100; i++) {
 *   for (j = 0; j < 100; j++) {
 *     stroke(i, j, 100);
 *     point(i, j);
 *   }
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * colorMode(RGB, 255);
 * var c = color(127, 255, 0);
 *
 * colorMode(RGB, 1);
 * var myColor = c._getRed();
 * text(myColor, 10, 10, 80, 80);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * colorMode(RGB, 255, 255, 255, 1);
 * background(255);
 *
 * strokeWeight(4);
 * stroke(255, 0 , 10, 0.3);
 * ellipse(40, 40, 50, 50);
 * ellipse(50, 50, 40, 40);
 * </code>
 * </div>
 *
 * @alt
 *Green to red gradient from bottom L to top R. shading originates from top left.
 *Rainbow gradient from left to right. Brightness increasing to white at top.
 *unknown image.
 *50x50 ellipse at middle L & 40x40 ellipse at center. Transluscent pink outlines.
 *
 */
p5.prototype.colorMode = function() {
  if (arguments[0] === constants.RGB ||
      arguments[0] === constants.HSB ||
      arguments[0] === constants.HSL) {

    // Set color mode.
    this._renderer._colorMode = arguments[0];

    // Set color maxes.
    var maxes = this._renderer._colorMaxes[this._renderer._colorMode];
    if (arguments.length === 2) {
      maxes[0] = arguments[1];  // Red
      maxes[1] = arguments[1];  // Green
      maxes[2] = arguments[1];  // Blue
      maxes[3] = arguments[1];  // Alpha
    } else if (arguments.length === 4) {
      maxes[0] = arguments[1];  // Red
      maxes[1] = arguments[2];  // Green
      maxes[2] = arguments[3];  // Blue
    } else if (arguments.length === 5) {
      maxes[0] = arguments[1];  // Red
      maxes[1] = arguments[2];  // Green
      maxes[2] = arguments[3];  // Blue
      maxes[3] = arguments[4];  // Alpha
    }
  }

  return this;
};

/**
 * Sets the color used to fill shapes. For example, if you run
 * fill(204, 102, 0), all subsequent shapes will be filled with orange. This
 * color is either specified in terms of the RGB or HSB color depending on
 * the current colorMode(). (The default color space is RGB, with each value
 * in the range from 0 to 255).
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color strings
 * and all named color strings are supported. In this case, an alpha number
 * value as a second argument is not supported, the RGBA form should be used.
 * <br><br>
 * A p5 Color object can also be provided to set the fill color.
 *
 * @method fill
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method fill
 * @param  {String}        value   a color string
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method fill
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 * @chainable
 */

/**
 * @method fill
 * @param  {p5.Color}      color   the fill color
 * @param  {Number}        [alpha]
 * @chainable
 *
 * @example
 * <div>
 * <code>
 * // Grayscale integer value
 * fill(51);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // R, G & B integer values
 * fill(255, 204, 0);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // H, S & B integer values
 * colorMode(HSB);
 * fill(255, 204, 100);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // Named SVG/CSS color string
 * fill('red');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // three-digit hexadecimal RGB notation
 * fill('#fae');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // six-digit hexadecimal RGB notation
 * fill('#222222');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGB notation
 * fill('rgb(0,255,0)');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGBA notation
 * fill('rgba(0,255,0, 0.25)');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGB notation
 * fill('rgb(100%,0%,10%)');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGBA notation
 * fill('rgba(100%,0%,100%,0.5)');
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // p5 Color object
 * fill(color(0, 0, 255));
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 * @alt
 * 60x60 dark charcoal grey rect with black outline in center of canvas.
 * 60x60 yellow rect with black outline in center of canvas.
 * 60x60 royal blue rect with black outline in center of canvas.
 * 60x60 red rect with black outline in center of canvas.
 * 60x60 pink rect with black outline in center of canvas.
 * 60x60 black rect with black outline in center of canvas.
 * 60x60 light green rect with black outline in center of canvas.
 * 60x60 soft green rect with black outline in center of canvas.
 * 60x60 red rect with black outline in center of canvas.
 * 60x60 dark fushcia rect with black outline in center of canvas.
 * 60x60 blue rect with black outline in center of canvas.
 */

p5.prototype.fill = function() {
  this._renderer._setProperty('_fillSet', true);
  this._renderer._setProperty('_doFill', true);
  this._renderer.fill.apply(this._renderer, arguments);
  return this;
};

/**
 * Disables filling geometry. If both noStroke() and noFill() are called,
 * nothing will be drawn to the screen.
 *
 * @method noFill
 * @chainable
 * @example
 * <div>
 * <code>
 * rect(15, 10, 55, 55);
 * noFill();
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 * @alt
 * white rect top middle and noFill rect center. Both 60x60 with black outlines.
 */
p5.prototype.noFill = function() {
  this._renderer._setProperty('_doFill', false);
  return this;
};

/**
 * Disables drawing the stroke (outline). If both noStroke() and noFill()
 * are called, nothing will be drawn to the screen.
 *
 * @method noStroke
 * @chainable
 * @example
 * <div>
 * <code>
 * noStroke();
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 *
 * @alt
 *60x60 white rect at center. no outline.
 *
 */

p5.prototype.noStroke = function() {
  this._renderer._setProperty('_doStroke', false);
  return this;
};

/**
 * Sets the color used to draw lines and borders around shapes. This color
 * is either specified in terms of the RGB or HSB color depending on the
 * current colorMode() (the default color space is RGB, with each value in
 * the range from 0 to 255).
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color
 * strings and all named color strings are supported. In this case, an alpha
 * number value as a second argument is not supported, the RGBA form should be
 * used.
 * <br><br>
 * A p5 Color object can also be provided to set the stroke color.
 *
 *
 * @method stroke
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method stroke
 * @param  {String}        value   a color string
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method stroke
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 * @chainable
 */

/**
 * @method stroke
 * @param  {p5.Color}      color   the stroke color
 * @param  {Number}        [alpha]
 * @chainable
 *
 * @example
 * <div>
 * <code>
 * // Grayscale integer value
 * strokeWeight(4);
 * stroke(51);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // R, G & B integer values
 * stroke(255, 204, 0);
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // H, S & B integer values
 * colorMode(HSB);
 * strokeWeight(4);
 * stroke(255, 204, 100);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // Named SVG/CSS color string
 * stroke('red');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // three-digit hexadecimal RGB notation
 * stroke('#fae');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // six-digit hexadecimal RGB notation
 * stroke('#222222');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGB notation
 * stroke('rgb(0,255,0)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // integer RGBA notation
 * stroke('rgba(0,255,0,0.25)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGB notation
 * stroke('rgb(100%,0%,10%)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // percentage RGBA notation
 * stroke('rgba(100%,0%,100%,0.5)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // p5 Color object
 * stroke(color(0, 0, 255));
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 * </code>
 * </div>
 *
 * @alt
 * 60x60 white rect at center. Dark charcoal grey outline.
 * 60x60 white rect at center. Yellow outline.
 * 60x60 white rect at center. Royal blue outline.
 * 60x60 white rect at center. Red outline.
 * 60x60 white rect at center. Pink outline.
 * 60x60 white rect at center. Black outline.
 * 60x60 white rect at center. Bright green outline.
 * 60x60 white rect at center. Soft green outline.
 * 60x60 white rect at center. Red outline.
 * 60x60 white rect at center. Dark fushcia outline.
 * 60x60 white rect at center. Blue outline.
 */

p5.prototype.stroke = function() {
  this._renderer._setProperty('_strokeSet', true);
  this._renderer._setProperty('_doStroke', true);
  this._renderer.stroke.apply(this._renderer, arguments);
  return this;
};

module.exports = p5;

},{"../core/constants":53,"../core/core":54,"./p5.Color":48}],50:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule 2D Primitives
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');
var canvas = _dereq_('./canvas');
_dereq_('./error_helpers');

/**
 * Draw an arc to the screen. If called with only a, b, c, d, start, and
 * stop, the arc will be drawn as an open pie. If mode is provided, the arc
 * will be drawn either open, as a chord, or as a pie as specified. The
 * origin may be changed with the ellipseMode() function.<br><br>
 * Note that drawing a full circle (ex: 0 to TWO_PI) will appear blank
 * because 0 and TWO_PI are the same position on the unit circle. The
 * best way to handle this is by using the ellipse() function instead
 * to create a closed ellipse, and to use the arc() function
 * only to draw parts of an ellipse.
 *
 * @method arc
 * @param  {Number} a      x-coordinate of the arc's ellipse
 * @param  {Number} b      y-coordinate of the arc's ellipse
 * @param  {Number} c      width of the arc's ellipse by default
 * @param  {Number} d      height of the arc's ellipse by default
 * @param  {Number} start  angle to start the arc, specified in radians
 * @param  {Number} stop   angle to stop the arc, specified in radians
 * @param  {Constant} [mode] optional parameter to determine the way of drawing
 *                         the arc. either CHORD or PIE
 * @chainable
 * @example
 * <div>
 * <code>
 * arc(50, 55, 50, 50, 0, HALF_PI);
 * noFill();
 * arc(50, 55, 60, 60, HALF_PI, PI);
 * arc(50, 55, 70, 70, PI, PI+QUARTER_PI);
 * arc(50, 55, 80, 80, PI+QUARTER_PI, TWO_PI);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * arc(50, 50, 80, 80, 0, PI+QUARTER_PI, OPEN);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * arc(50, 50, 80, 80, 0, PI+QUARTER_PI, CHORD);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * arc(50, 50, 80, 80, 0, PI+QUARTER_PI, PIE);
 * </code>
 * </div>
 *
 * @alt
 *shattered outline of an ellipse with a quarter of a white circle bottom-right.
 *white ellipse with black outline with top right missing.
 *white ellipse with top right missing with black outline around shape.
 *white ellipse with top right quarter missing with black outline around the shape.
 *
 */
p5.prototype.arc = function(x, y, w, h, start, stop, mode) {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  if (this._angleMode === constants.DEGREES) {
    start = this.radians(start);
    stop = this.radians(stop);
  }

  // Make all angles positive...
  while (start < 0) {
    start += constants.TWO_PI;
  }
  while (stop < 0) {
    stop += constants.TWO_PI;
  }
  // ...and confine them to the interval [0,TWO_PI).
  start %= constants.TWO_PI;
  stop %= constants.TWO_PI;

  // account for full circle
  if (stop === start) {
    stop += constants.TWO_PI;
  }

  // Adjust angles to counter linear scaling.
  if (start <= constants.HALF_PI) {
    start = Math.atan(w / h * Math.tan(start));
  } else  if (start > constants.HALF_PI && start <= 3 * constants.HALF_PI) {
    start = Math.atan(w / h * Math.tan(start)) + constants.PI;
  } else {
    start = Math.atan(w / h * Math.tan(start)) + constants.TWO_PI;
  }
  if (stop <= constants.HALF_PI) {
    stop = Math.atan(w / h * Math.tan(stop));
  } else  if (stop > constants.HALF_PI && stop <= 3 * constants.HALF_PI) {
    stop = Math.atan(w / h * Math.tan(stop)) + constants.PI;
  } else {
    stop = Math.atan(w / h * Math.tan(stop)) + constants.TWO_PI;
  }

  // Exceed the interval if necessary in order to preserve the size and
  // orientation of the arc.
  if (start > stop) {
    stop += constants.TWO_PI;
  }
  // p5 supports negative width and heights for ellipses
  w = Math.abs(w);
  h = Math.abs(h);
  this._renderer.arc(x, y, w, h, start, stop, mode);
  return this;
};

/**
 * Draws an ellipse (oval) to the screen. An ellipse with equal width and
 * height is a circle. By default, the first two parameters set the location,
 * and the third and fourth parameters set the shape's width and height. If
 * no height is specified, the value of width is used for both the width and
 * height. If a negative height or width is specified, the absolute value is taken.
 * The origin may be changed with the ellipseMode() function.
 *
 * @method ellipse
 * @param  {Number} x x-coordinate of the ellipse.
 * @param  {Number} y y-coordinate of the ellipse.
 * @param  {Number} w width of the ellipse.
 * @param  {Number} [h] height of the ellipse.
 * @chainable
 * @example
 * <div>
 * <code>
 * ellipse(56, 46, 55, 55);
 * </code>
 * </div>
 *
 * @alt
 *white ellipse with black outline in middle-right of canvas that is 55x55.
 *
 */
p5.prototype.ellipse = function() {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  // Duplicate 3rd argument if only 3 given.
  if (args.length === 3) {
    args.push(args[2]);
  }
  // p5 supports negative width and heights for rects
  if (args[2] < 0){args[2] = Math.abs(args[2]);}
  if (args[3] < 0){args[3] = Math.abs(args[3]);}
  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  var vals = canvas.modeAdjust(
    args[0],
    args[1],
    args[2],
    args[3],
    this._renderer._ellipseMode);
  args[0] = vals.x;
  args[1] = vals.y;
  args[2] = vals.w;
  args[3] = vals.h;
  this._renderer.ellipse(args);
  return this;
};
/**
 * Draws a line (a direct path between two points) to the screen. The version
 * of line() with four parameters draws the line in 2D. To color a line, use
 * the stroke() function. A line cannot be filled, therefore the fill()
 * function will not affect the color of a line. 2D lines are drawn with a
 * width of one pixel by default, but this can be changed with the
 * strokeWeight() function.
 *
 * @method line
 * @param  {Number} x1 the x-coordinate of the first point
 * @param  {Number} y1 the y-coordinate of the first point
 * @param  {Number} x2 the x-coordinate of the second point
 * @param  {Number} y2 the y-coordinate of the second point
 * @chainable
 * @example
 * <div>
 * <code>
 * line(30, 20, 85, 75);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * line(30, 20, 85, 20);
 * stroke(126);
 * line(85, 20, 85, 75);
 * stroke(255);
 * line(85, 75, 30, 75);
 * </code>
 * </div>
 *
 * @alt
 *line 78 pixels long running from mid-top to bottom-right of canvas.
 *3 lines of various stroke sizes. Form top, bottom and right sides of a square.
 *
 */
////commented out original
// p5.prototype.line = function(x1, y1, x2, y2) {
//   if (!this._renderer._doStroke) {
//     return this;
//   }
//   if(this._renderer.isP3D){
//   } else {
//     this._renderer.line(x1, y1, x2, y2);
//   }
// };
p5.prototype.line = function() {
  if (!this._renderer._doStroke) {
    return this;
  }
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  //check whether we should draw a 3d line or 2d
  if(this._renderer.isP3D){
    this._renderer.line(
      args[0],
      args[1],
      args[2],
      args[3],
      args[4],
      args[5]);
  } else {
    this._renderer.line(
      args[0],
      args[1],
      args[2],
      args[3]);
  }
  return this;
};

/**
 * Draws a point, a coordinate in space at the dimension of one pixel.
 * The first parameter is the horizontal value for the point, the second
 * value is the vertical value for the point. The color of the point is
 * determined by the current stroke.
 *
 * @method point
 * @param  {Number} x the x-coordinate
 * @param  {Number} y the y-coordinate
 * @chainable
 * @example
 * <div>
 * <code>
 * point(30, 20);
 * point(85, 20);
 * point(85, 75);
 * point(30, 75);
 * </code>
 * </div>
 *
 * @alt
 *4 points centered in the middle-right of the canvas.
 *
 */
p5.prototype.point = function() {
  if (!this._renderer._doStroke) {
    return this;
  }
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  //check whether we should draw a 3d line or 2d
  if(this._renderer.isP3D){
    this._renderer.point(
      args[0],
      args[1],
      args[2]
      );
  } else {
    this._renderer.point(
      args[0],
      args[1]
    );
  }
  return this;
};


/**
 * Draw a quad. A quad is a quadrilateral, a four sided polygon. It is
 * similar to a rectangle, but the angles between its edges are not
 * constrained to ninety degrees. The first pair of parameters (x1,y1)
 * sets the first vertex and the subsequent pairs should proceed
 * clockwise or counter-clockwise around the defined shape.
 *
 * @method quad
 * @param {Number} x1 the x-coordinate of the first point
 * @param {Number} y1 the y-coordinate of the first point
 * @param {Number} x2 the x-coordinate of the second point
 * @param {Number} y2 the y-coordinate of the second point
 * @param {Number} x3 the x-coordinate of the third point
 * @param {Number} y3 the y-coordinate of the third point
 * @param {Number} x4 the x-coordinate of the fourth point
 * @param {Number} y4 the y-coordinate of the fourth point
 * @chainable
 * @example
 * <div>
 * <code>
 * quad(38, 31, 86, 20, 69, 63, 30, 76);
 * </code>
 * </div>
 *
 * @alt
 *irregular white quadrilateral shape with black outline mid-right of canvas.
 *
 */
/**
 * @method quad
 * @param {Number} x1
 * @param {Number} y1
 * @param {Number} x2
 * @param {Number} y2
 * @param {Number} x3
 * @param {Number} y3
 * @param {Number} x4
 * @param {Number} y4
 * @chainable
 */
p5.prototype.quad = function() {
  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if(this._renderer.isP3D){
    this._renderer.quad(
      args[0],
      args[1],
      args[2],
      args[3],
      args[4],
      args[5],
      args[6],
      args[7],
      args[8],
      args[9],
      args[10],
      args[11]
      );
  } else {
    this._renderer.quad(
     args[0],
     args[1],
     args[2],
     args[3],
     args[4],
     args[5],
     args[6],
    args[7]
    );
  }
  return this;
};

/**
* Draws a rectangle to the screen. A rectangle is a four-sided shape with
* every angle at ninety degrees. By default, the first two parameters set
* the location of the upper-left corner, the third sets the width, and the
* fourth sets the height. The way these parameters are interpreted, however,
* may be changed with the rectMode() function.
* <br><br>
* The fifth, sixth, seventh and eighth parameters, if specified,
* determine corner radius for the top-right, top-left, lower-right and
* lower-left corners, respectively. An omitted corner radius parameter is set
* to the value of the previously specified radius value in the parameter list.
*
* @method rect
* @param  {Number} x  x-coordinate of the rectangle.
* @param  {Number} y  y-coordinate of the rectangle.
* @param  {Number} w  width of the rectangle.
* @param  {Number} h  height of the rectangle.
* @param  {Number} [tl] optional radius of top-left corner.
* @param  {Number} [tr] optional radius of top-right corner.
* @param  {Number} [br] optional radius of bottom-right corner.
* @param  {Number} [bl] optional radius of bottom-left corner.
* @return {p5}          the p5 object.
* @example
* <div>
* <code>
* // Draw a rectangle at location (30, 20) with a width and height of 55.
* rect(30, 20, 55, 55);
* </code>
* </div>
*
* <div>
* <code>
* // Draw a rectangle with rounded corners, each having a radius of 20.
* rect(30, 20, 55, 55, 20);
* </code>
* </div>
*
* <div>
* <code>
* // Draw a rectangle with rounded corners having the following radii:
* // top-left = 20, top-right = 15, bottom-right = 10, bottom-left = 5.
* rect(30, 20, 55, 55, 20, 15, 10, 5);
* </code>
* </div>
*
* @alt
* 55x55 white rect with black outline in mid-right of canvas.
* 55x55 white rect with black outline and rounded edges in mid-right of canvas.
* 55x55 white rect with black outline and rounded edges of different radii.
*/
/**
* @method rect
* @param  {Number} x
* @param  {Number} y
* @param  {Number} w
* @param  {Number} h
* @param  {Number} [detailX]
* @param  {Number} [detailY]
* @chainable
*/
p5.prototype.rect = function () {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  var vals = canvas.modeAdjust(
    args[0],
    args[1],
    args[2],
    args[3],
    this._renderer._rectMode);
  args[0] = vals.x;
  args[1] = vals.y;
  args[2] = vals.w;
  args[3] = vals.h;
  this._renderer.rect(args);
  return this;
};

/**
* A triangle is a plane created by connecting three points. The first two
* arguments specify the first point, the middle two arguments specify the
* second point, and the last two arguments specify the third point.
*
* @method triangle
* @param  {Number} x1 x-coordinate of the first point
* @param  {Number} y1 y-coordinate of the first point
* @param  {Number} x2 x-coordinate of the second point
* @param  {Number} y2 y-coordinate of the second point
* @param  {Number} x3 x-coordinate of the third point
* @param  {Number} y3 y-coordinate of the third point
* @chainable
* @example
* <div>
* <code>
* triangle(30, 75, 58, 20, 86, 75);
* </code>
* </div>
*
*@alt
* white triangle with black outline in mid-right of canvas.
*
*/
p5.prototype.triangle = function() {

  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  this._renderer.triangle(args);
  return this;
};

module.exports = p5;

},{"./canvas":52,"./constants":53,"./core":54,"./error_helpers":57}],51:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule Attributes
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');

/**
 * Modifies the location from which ellipses are drawn by changing the way
 * in which parameters given to ellipse() are interpreted.
 * <br><br>
 * The default mode is ellipseMode(CENTER), which interprets the first two
 * parameters of ellipse() as the shape's center point, while the third and
 * fourth parameters are its width and height.
 * <br><br>
 * ellipseMode(RADIUS) also uses the first two parameters of ellipse() as
 * the shape's center point, but uses the third and fourth parameters to
 * specify half of the shapes's width and height.
 * <br><br>
 * ellipseMode(CORNER) interprets the first two parameters of ellipse() as
 * the upper-left corner of the shape, while the third and fourth parameters
 * are its width and height.
 * <br><br>
 * ellipseMode(CORNERS) interprets the first two parameters of ellipse() as
 * the location of one corner of the ellipse's bounding box, and the third
 * and fourth parameters as the location of the opposite corner.
 * <br><br>
 * The parameter must be written in ALL CAPS because Javascript is a
 * case-sensitive language.
 *
 * @method ellipseMode
 * @param  {Constant} mode either CENTER, RADIUS, CORNER, or CORNERS
 * @chainable
 * @example
 * <div>
 * <code>
 * ellipseMode(RADIUS);  // Set ellipseMode to RADIUS
 * fill(255);  // Set fill to white
 * ellipse(50, 50, 30, 30);  // Draw white ellipse using RADIUS mode
 *
 * ellipseMode(CENTER);  // Set ellipseMode to CENTER
 * fill(100);  // Set fill to gray
 * ellipse(50, 50, 30, 30);  // Draw gray ellipse using CENTER mode
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * ellipseMode(CORNER);  // Set ellipseMode is CORNER
 * fill(255);  // Set fill to white
 * ellipse(25, 25, 50, 50);  // Draw white ellipse using CORNER mode
 *
 * ellipseMode(CORNERS);  // Set ellipseMode to CORNERS
 * fill(100);  // Set fill to gray
 * ellipse(25, 25, 50, 50);  // Draw gray ellipse using CORNERS mode
 * </code>
 * </div>
 *
 * @alt
 * 60x60 white ellipse and 30x30 grey ellipse with black outlines at center.
 * 60x60 white ellipse @center and 30x30 grey ellipse top-right, black outlines.
 *
 */
p5.prototype.ellipseMode = function(m) {
  if (m === constants.CORNER ||
    m === constants.CORNERS ||
    m === constants.RADIUS ||
    m === constants.CENTER) {
    this._renderer._ellipseMode = m;
  }
  return this;
};

/**
 * Draws all geometry with jagged (aliased) edges. Note that smooth() is
 * active by default, so it is necessary to call noSmooth() to disable
 * smoothing of geometry, images, and fonts.
 *
 * @method noSmooth
 * @chainable
 * @example
 * <div>
 * <code>
 * background(0);
 * noStroke();
 * smooth();
 * ellipse(30, 48, 36, 36);
 * noSmooth();
 * ellipse(70, 48, 36, 36);
 * </code>
 * </div>
 *
 * @alt
 * 2 pixelated 36x36 white ellipses to left & right of center, black background
 *
 */
p5.prototype.noSmooth = function() {
  this._renderer.noSmooth();
  return this;
};

/**
 * Modifies the location from which rectangles are drawn by changing the way
 * in which parameters given to rect() are interpreted.
 * <br><br>
 * The default mode is rectMode(CORNER), which interprets the first two
 * parameters of rect() as the upper-left corner of the shape, while the
 * third and fourth parameters are its width and height.
 * <br><br>
 * rectMode(CORNERS) interprets the first two parameters of rect() as the
 * location of one corner, and the third and fourth parameters as the
 * location of the opposite corner.
 * <br><br>
 * rectMode(CENTER) interprets the first two parameters of rect() as the
 * shape's center point, while the third and fourth parameters are its
 * width and height.
 * <br><br>
 * rectMode(RADIUS) also uses the first two parameters of rect() as the
 * shape's center point, but uses the third and fourth parameters to specify
 * half of the shapes's width and height.
 * <br><br>
 * The parameter must be written in ALL CAPS because Javascript is a
 * case-sensitive language.
 *
 * @method rectMode
 * @param  {Constant} mode either CORNER, CORNERS, CENTER, or RADIUS
 * @chainable
 * @example
 * <div>
 * <code>
 * rectMode(CORNER);  // Default rectMode is CORNER
 * fill(255);  // Set fill to white
 * rect(25, 25, 50, 50);  // Draw white rect using CORNER mode
 *
 * rectMode(CORNERS);  // Set rectMode to CORNERS
 * fill(100);  // Set fill to gray
 * rect(25, 25, 50, 50);  // Draw gray rect using CORNERS mode
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * rectMode(RADIUS);  // Set rectMode to RADIUS
 * fill(255);  // Set fill to white
 * rect(50, 50, 30, 30);  // Draw white rect using RADIUS mode
 *
 * rectMode(CENTER);  // Set rectMode to CENTER
 * fill(100);  // Set fill to gray
 * rect(50, 50, 30, 30);  // Draw gray rect using CENTER mode
 * </code>
 * </div>
 *
 * @alt
 * 50x50 white rect at center and 25x25 grey rect in the top left of the other.
 * 50x50 white rect at center and 25x25 grey rect in the center of the other.
 *
 */
p5.prototype.rectMode = function(m) {
  if (m === constants.CORNER ||
    m === constants.CORNERS ||
    m === constants.RADIUS ||
    m === constants.CENTER) {
    this._renderer._rectMode = m;
  }
  return this;
};

/**
 * Draws all geometry with smooth (anti-aliased) edges. smooth() will also
 * improve image quality of resized images. Note that smooth() is active by
 * default; noSmooth() can be used to disable smoothing of geometry,
 * images, and fonts.
 *
 * @method smooth
 * @chainable
 * @example
 * <div>
 * <code>
 * background(0);
 * noStroke();
 * smooth();
 * ellipse(30, 48, 36, 36);
 * noSmooth();
 * ellipse(70, 48, 36, 36);
 * </code>
 * </div>
 *
 * @alt
 * 2 pixelated 36x36 white ellipses one left one right of center. On black.
 *
 */
p5.prototype.smooth = function() {
  this._renderer.smooth();
  return this;
};

/**
 * Sets the style for rendering line endings. These ends are either squared,
 * extended, or rounded, each of which specified with the corresponding
 * parameters: SQUARE, PROJECT, and ROUND. The default cap is ROUND.
 *
 * @method strokeCap
 * @param  {Constant} cap either SQUARE, PROJECT, or ROUND
 * @chainable
 * @example
 * <div>
 * <code>
 * strokeWeight(12.0);
 * strokeCap(ROUND);
 * line(20, 30, 80, 30);
 * strokeCap(SQUARE);
 * line(20, 50, 80, 50);
 * strokeCap(PROJECT);
 * line(20, 70, 80, 70);
 * </code>
 * </div>
 *
 * @alt
 * 3 lines. Top line: rounded ends, mid: squared, bottom:longer squared ends.
 *
 */
p5.prototype.strokeCap = function(cap) {
  if (cap === constants.ROUND ||
    cap === constants.SQUARE ||
    cap === constants.PROJECT) {
    this._renderer.strokeCap(cap);
  }
  return this;
};

/**
 * Sets the style of the joints which connect line segments. These joints
 * are either mitered, beveled, or rounded and specified with the
 * corresponding parameters MITER, BEVEL, and ROUND. The default joint is
 * MITER.
 *
 * @method strokeJoin
 * @param  {Constant} join either MITER, BEVEL, ROUND
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 * strokeWeight(10.0);
 * strokeJoin(MITER);
 * beginShape();
 * vertex(35, 20);
 * vertex(65, 50);
 * vertex(35, 80);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * strokeWeight(10.0);
 * strokeJoin(BEVEL);
 * beginShape();
 * vertex(35, 20);
 * vertex(65, 50);
 * vertex(35, 80);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * strokeWeight(10.0);
 * strokeJoin(ROUND);
 * beginShape();
 * vertex(35, 20);
 * vertex(65, 50);
 * vertex(35, 80);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * Right-facing arrowhead shape with pointed tip in center of canvas.
 * Right-facing arrowhead shape with flat tip in center of canvas.
 * Right-facing arrowhead shape with rounded tip in center of canvas.
 *
 */
p5.prototype.strokeJoin = function(join) {
  if (join === constants.ROUND ||
    join === constants.BEVEL ||
    join === constants.MITER) {
    this._renderer.strokeJoin(join);
  }
  return this;
};

/**
 * Sets the width of the stroke used for lines, points, and the border
 * around shapes. All widths are set in units of pixels.
 *
 * @method strokeWeight
 * @param  {Number} weight the weight (in pixels) of the stroke
 * @return {p5}            the p5 object
 * @example
 * <div>
 * <code>
 * strokeWeight(1);  // Default
 * line(20, 20, 80, 20);
 * strokeWeight(4);  // Thicker
 * line(20, 40, 80, 40);
 * strokeWeight(10);  // Beastly
 * line(20, 70, 80, 70);
 * </code>
 * </div>
 *
 * @alt
 * 3 horizontal black lines. Top line: thin, mid: medium, bottom:thick.
 *
 */
p5.prototype.strokeWeight = function(w) {
  this._renderer.strokeWeight(w);
  return this;
};

module.exports = p5;

},{"./constants":53,"./core":54}],52:[function(_dereq_,module,exports){
/**
 * @requires constants
 */

var constants = _dereq_('./constants');

module.exports = {

  modeAdjust: function(a, b, c, d, mode) {
    if (mode === constants.CORNER) {
      return { x: a, y: b, w: c, h: d };
    } else if (mode === constants.CORNERS) {
      return { x: a, y: b, w: c-a, h: d-b };
    } else if (mode === constants.RADIUS) {
      return { x: a-c, y: b-d, w: 2*c, h: 2*d };
    } else if (mode === constants.CENTER) {
      return { x: a-c*0.5, y: b-d*0.5, w: c, h: d };
    }
  },

  arcModeAdjust: function(a, b, c, d, mode) {
    if (mode === constants.CORNER) {
      return { x: a+c*0.5, y: b+d*0.5, w: c, h: d };
    } else if (mode === constants.CORNERS) {
      return { x: a, y: b, w: c+a, h: d+b };
    } else if (mode === constants.RADIUS) {
      return { x: a, y: b, w: 2*c, h: 2*d };
    } else if (mode === constants.CENTER) {
      return { x: a, y: b, w: c, h: d };
    }
  }

};


},{"./constants":53}],53:[function(_dereq_,module,exports){
/**
 * @module Constants
 * @submodule Constants
 * @for p5
 */

var PI = Math.PI;

module.exports = {

  // GRAPHICS RENDERER
  /**
   * @property {String} P2D
   * @final
   */
  P2D: 'p2d',
  /**
   * @property {String} WEBGL
   * @final
   */
  WEBGL: 'webgl',

  // ENVIRONMENT
  ARROW: 'default',
  CROSS: 'crosshair',
  HAND: 'pointer',
  MOVE: 'move',
  TEXT: 'text',
  WAIT: 'wait',

  // TRIGONOMETRY

  /**
   * HALF_PI is a mathematical constant with the value
   * 1.57079632679489661923. It is half the ratio of the
   * circumference of a circle to its diameter. It is useful in
   * combination with the trigonometric functions sin() and cos().
   *
   * @property {Number} HALF_PI
   * @final
   *
   * @example
   * <div><code>
   * arc(50, 50, 80, 80, 0, HALF_PI);
   * </code></div>
   *
   * @alt
   * 80x80 white quarter-circle with curve toward bottom right of canvas.
   *
   */
  HALF_PI: PI / 2,
  /**
   * PI is a mathematical constant with the value
   * 3.14159265358979323846. It is the ratio of the circumference
   * of a circle to its diameter. It is useful in combination with
   * the trigonometric functions sin() and cos().
   *
   * @property {Number} PI
   * @final
   *
   * @example
   * <div><code>
   * arc(50, 50, 80, 80, 0, PI);
   * </code></div>
   *
   * @alt
   * white half-circle with curve toward bottom of canvas.
   *
   */
  PI: PI,
  /**
   * QUARTER_PI is a mathematical constant with the value 0.7853982.
   * It is one quarter the ratio of the circumference of a circle to
   * its diameter. It is useful in combination with the trigonometric
   * functions sin() and cos().
   *
   * @property {Number} QUARTER_PI
   * @final
   *
   * @example
   * <div><code>
   * arc(50, 50, 80, 80, 0, QUARTER_PI);
   * </code></div>
   *
   * @alt
   * white eighth-circle rotated about 40 degrees with curve bottom right canvas.
   *
   */
  QUARTER_PI: PI / 4,
  /**
   * TAU is an alias for TWO_PI, a mathematical constant with the
   * value 6.28318530717958647693. It is twice the ratio of the
   * circumference of a circle to its diameter. It is useful in
   * combination with the trigonometric functions sin() and cos().
   *
   * @property {Number} TAU
   * @final
   *
   * @example
   * <div><code>
   * arc(50, 50, 80, 80, 0, TAU);
   * </code></div>
   *
   * @alt
   * 80x80 white ellipse shape in center of canvas.
   *
   */
  TAU: PI * 2,
  /**
   * TWO_PI is a mathematical constant with the value
   * 6.28318530717958647693. It is twice the ratio of the
   * circumference of a circle to its diameter. It is useful in
   * combination with the trigonometric functions sin() and cos().
   *
   * @property {Number} TWO_PI
   * @final
   *
   * @example
   * <div><code>
   * arc(50, 50, 80, 80, 0, TWO_PI);
   * </code></div>
   *
   * @alt
   * 80x80 white ellipse shape in center of canvas.
   *
   */
  TWO_PI: PI * 2,
  /**
   * @property {String} DEGREES
   * @final
   */
  DEGREES: 'degrees',
  /**
   * @property {String} RADIANS
   * @final
   */
  RADIANS: 'radians',
  DEG_TO_RAD: PI / 180.0,
  RAD_TO_DEG: 180.0 / PI,

  // SHAPE
  /**
   * @property {String} CORNER
   * @final
   */
  CORNER: 'corner',
  /**
   * @property {String} CORNERS
   * @final
   */
  CORNERS: 'corners',
  /**
   * @property {String} RADIUS
   * @final
   */
  RADIUS: 'radius',
  /**
   * @property {String} RIGHT
   * @final
   */
  RIGHT: 'right',
  /**
   * @property {String} LEFT
   * @final
   */
  LEFT: 'left',
  /**
   * @property {String} CENTER
   * @final
   */
  CENTER: 'center',
  /**
   * @property {String} TOP
   * @final
   */
  TOP: 'top',
  /**
   * @property {String} BOTTOM
   * @final
   */
  BOTTOM: 'bottom',
  /**
   * @property {String} BASELINE
   * @final
   * @default alphabetic
   */
  BASELINE: 'alphabetic',
  /**
   * @property {Number} POINTS
   * @final
   * @default 0x0000
   */
  POINTS: 0x0000,
  /**
   * @property {Number} LINES
   * @final
   * @default 0x0001
   */
  LINES: 0x0001,
  /**
   * @property {Number} LINE_STRIP
   * @final
   * @default 0x0003
   */
  LINE_STRIP: 0x0003,
  /**
   * @property {Number} LINE_LOOP
   * @final
   * @default 0x0002
   */
  LINE_LOOP: 0x0002,
  /**
   * @property {Number} TRIANGLES
   * @final
   * @default 0x0004
   */
  TRIANGLES: 0x0004,
  /**
   * @property {Number} TRIANGLE_FAN
   * @final
   * @default 0x0006
   */
  TRIANGLE_FAN: 0x0006,
  /**
   * @property {Number} TRIANGLE_STRIP
   * @final
   * @default 0x0005
   */
  TRIANGLE_STRIP: 0x0005,
  /**
   * @property {String} QUADS
   * @final
   */
  QUADS: 'quads',
  /**
   * @property {String} QUAD_STRIP
   * @final
   * @default quad_strip
   */
  QUAD_STRIP: 'quad_strip',
  /**
   * @property {String} CLOSE
   * @final
   */
  CLOSE: 'close',
  /**
   * @property {String} OPEN
   * @final
   */
  OPEN: 'open',
  /**
   * @property {String} CHORD
   * @final
   */
  CHORD: 'chord',
  /**
   * @property {String} PIE
   * @final
   */
  PIE: 'pie',
  /**
   * @property {String} PROJECT
   * @final
   * @default square
   */
  PROJECT: 'square', // PEND: careful this is counterintuitive
  /**
   * @property {String} SQUARE
   * @final
   * @default butt
   */
  SQUARE: 'butt',
  /**
   * @property {String} ROUND
   * @final
   */
  ROUND: 'round',
  /**
   * @property {String} BEVEL
   * @final
   */
  BEVEL: 'bevel',
  /**
   * @property {String} MITER
   * @final
   */
  MITER: 'miter',

  // COLOR
  /**
   * @property {String} RGB
   * @final
   */
  RGB: 'rgb',
  /**
   * @property {String} HSB
   * @final
   */
  HSB: 'hsb',
  /**
   * @property {String} HSL
   * @final
   */
  HSL: 'hsl',

  // DOM EXTENSION
  AUTO: 'auto',

  // INPUT
  ALT: 18,
  BACKSPACE: 8,
  CONTROL: 17,
  DELETE: 46,
  DOWN_ARROW: 40,
  ENTER: 13,
  ESCAPE: 27,
  LEFT_ARROW: 37,
  OPTION: 18,
  RETURN: 13,
  RIGHT_ARROW: 39,
  SHIFT: 16,
  TAB: 9,
  UP_ARROW: 38,

  // RENDERING
  /**
   * @property {String} BLEND
   * @final
   * @default source-over
   */
  BLEND: 'source-over',
  /**
   * @property {String} ADD
   * @final
   * @default lighter
   */
  ADD: 'lighter',
  //ADD: 'add', //
  //SUBTRACT: 'subtract', //
  /**
   * @property {String} DARKEST
   * @final
   */
  DARKEST: 'darken',
  /**
   * @property {String} LIGHTEST
   * @final
   * @default lighten
   */
  LIGHTEST: 'lighten',
  /**
   * @property {String} DIFFERENCE
   * @final
   */
  DIFFERENCE: 'difference',
  /**
   * @property {String} EXCLUSION
   * @final
   */
  EXCLUSION: 'exclusion',
  /**
   * @property {String} MULTIPLY
   * @final
   */
  MULTIPLY: 'multiply',
  /**
   * @property {String} SCREEN
   * @final
   */
  SCREEN: 'screen',
  /**
   * @property {String} REPLACE
   * @final
   * @default copy
   */
  REPLACE: 'copy',
  /**
   * @property {String} OVERLAY
   * @final
   */
  OVERLAY: 'overlay',
  /**
   * @property {String} HARD_LIGHT
   * @final
   */
  HARD_LIGHT: 'hard-light',
  /**
   * @property {String} SOFT_LIGHT
   * @final
   */
  SOFT_LIGHT: 'soft-light',
  /**
   * @property {String} DODGE
   * @final
   * @default color-dodge
   */
  DODGE: 'color-dodge',
  /**
   * @property {String} BURN
   * @final
   * @default color-burn
   */
  BURN: 'color-burn',

  // FILTERS
  /**
   * @property {String} THRESHOLD
   * @final
   */
  THRESHOLD: 'threshold',
  /**
   * @property {String} GRAY
   * @final
   */
  GRAY: 'gray',
  /**
   * @property {String} OPAQUE
   * @final
   */
  OPAQUE: 'opaque',
  /**
   * @property {String} INVERT
   * @final
   */
  INVERT: 'invert',
  /**
   * @property {String} POSTERIZE
   * @final
   */
  POSTERIZE: 'posterize',
  /**
   * @property {String} DILATE
   * @final
   */
  DILATE: 'dilate',
  /**
   * @property {String} ERODE
   * @final
   */
  ERODE: 'erode',
  /**
   * @property {String} BLUR
   * @final
   */
  BLUR: 'blur',

  // TYPOGRAPHY
  /**
   * @property {String} NORMAL
   * @final
   */
  NORMAL: 'normal',
  /**
   * @property {String} ITALIC
   * @final
   */
  ITALIC: 'italic',
  /**
   * @property {String} BOLD
   * @final
   */
  BOLD: 'bold',

  // TYPOGRAPHY-INTERNAL
  _DEFAULT_TEXT_FILL: '#000000',
  _DEFAULT_LEADMULT: 1.25,
  _CTX_MIDDLE: 'middle',

  // VERTICES
  LINEAR: 'linear',
  QUADRATIC: 'quadratic',
  BEZIER: 'bezier',
  CURVE: 'curve',

  // DEVICE-ORIENTATION
  /**
   * @property {String} LANDSCAPE
   * @final
   */
  LANDSCAPE: 'landscape',
  /**
   * @property {String} PORTRAIT
   * @final
   */
  PORTRAIT: 'portrait',

  // DEFAULTS
  _DEFAULT_STROKE: '#000000',
  _DEFAULT_FILL: '#FFFFFF'

};

},{}],54:[function(_dereq_,module,exports){
/**
 * @module Structure
 * @submodule Structure
 * @for p5
 * @requires constants
 */

'use strict';

_dereq_('./shim');

// Core needs the PVariables object
var constants = _dereq_('./constants');

/**
 * This is the p5 instance constructor.
 *
 * A p5 instance holds all the properties and methods related to
 * a p5 sketch.  It expects an incoming sketch closure and it can also
 * take an optional node parameter for attaching the generated p5 canvas
 * to a node.  The sketch closure takes the newly created p5 instance as
 * its sole argument and may optionally set preload(), setup(), and/or
 * draw() properties on it for running a sketch.
 *
 * A p5 sketch can run in "global" or "instance" mode:
 * "global"   - all properties and methods are attached to the window
 * "instance" - all properties and methods are bound to this p5 object
 *
 * @param  {function}    sketch a closure that can set optional preload(),
 *                              setup(), and/or draw() properties on the
 *                              given p5 instance
 * @param  {HTMLElement|boolean} [node] element to attach canvas to, if a
 *                                      boolean is passed in use it as sync
 * @param  {boolean}     [sync] start synchronously (optional)
 * @return {p5}                 a p5 instance
 */
var p5 = function(sketch, node, sync) {

  if (arguments.length === 2 && typeof node === 'boolean') {
    sync = node;
    node = undefined;
  }

  //////////////////////////////////////////////
  // PUBLIC p5 PROPERTIES AND METHODS
  //////////////////////////////////////////////


  /**
   * Called directly before setup(), the preload() function is used to handle
   * asynchronous loading of external files. If a preload function is
   * defined, setup() will wait until any load calls within have finished.
   * Nothing besides load calls should be inside preload (loadImage,
   * loadJSON, loadFont, loadStrings, etc).<br><br>
   * By default the text "loading..." will be displayed. To make your own
   * loading page, include an HTML element with id "p5_loading" in your
   * page. More information <a href="http://bit.ly/2kQ6Nio">here</a>.
   *
   * @method preload
   * @example
   * <div><code>
   * var img;
   * var c;
   * function preload() {  // preload() runs once
   *   img = loadImage('assets/laDefense.jpg');
   * }
   *
   * function setup() {  // setup() waits until preload() is done
   *   img.loadPixels();
   *   // get color of middle pixel
   *   c = img.get(img.width/2, img.height/2);
   * }
   *
   * function draw() {
   *   background(c);
   *   image(img, 25, 25, 50, 50);
   * }
   * </code></div>
   *
   * @alt
   * nothing displayed
   *
   */

  /**
   * The setup() function is called once when the program starts. It's used to
   * define initial environment properties such as screen size and background
   * color and to load media such as images and fonts as the program starts.
   * There can only be one setup() function for each program and it shouldn't
   * be called again after its initial execution.
   * <br><br>
   * Note: Variables declared within setup() are not accessible within other
   * functions, including draw().
   *
   * @method setup
   * @example
   * <div><code>
   * var a = 0;
   *
   * function setup() {
   *   background(0);
   *   noStroke();
   *   fill(102);
   * }
   *
   * function draw() {
   *   rect(a++%width, 10, 2, 80);
   * }
   * </code></div>
   *
   * @alt
   * nothing displayed
   *
   */

  /**
   * Called directly after setup(), the draw() function continuously executes
   * the lines of code contained inside its block until the program is stopped
   * or noLoop() is called. Note if noLoop() is called in setup(), draw() will
   * still be executed once before stopping. draw() is called automatically and
   * should never be called explicitly.
   * <br><br>
   * It should always be controlled with noLoop(), redraw() and loop(). After
   * noLoop() stops the code in draw() from executing, redraw() causes the
   * code inside draw() to execute once, and loop() will cause the code
   * inside draw() to resume executing continuously.
   * <br><br>
   * The number of times draw() executes in each second may be controlled with
   * the frameRate() function.
   * <br><br>
   * There can only be one draw() function for each sketch, and draw() must
   * exist if you want the code to run continuously, or to process events such
   * as mousePressed(). Sometimes, you might have an empty call to draw() in
   * your program, as shown in the above example.
   * <br><br>
   * It is important to note that the drawing coordinate system will be reset
   * at the beginning of each draw() call. If any transformations are performed
   * within draw() (ex: scale, rotate, translate, their effects will be
   * undone at the beginning of draw(), so transformations will not accumulate
   * over time. On the other hand, styling applied (ex: fill, stroke, etc) will
   * remain in effect.
   *
   * @method draw
   * @example
   * <div><code>
   * var yPos = 0;
   * function setup() {  // setup() runs once
   *   frameRate(30);
   * }
   * function draw() {  // draw() loops forever, until stopped
   *   background(204);
   *   yPos = yPos - 1;
   *   if (yPos < 0) {
   *     yPos = height;
   *   }
   *   line(0, yPos, width, yPos);
   * }
   * </code></div>
   *
   * @alt
   * nothing displayed
   *
   */


  //////////////////////////////////////////////
  // PRIVATE p5 PROPERTIES AND METHODS
  //////////////////////////////////////////////

  this._setupDone = false;
  // for handling hidpi
  this._pixelDensity = Math.ceil(window.devicePixelRatio) || 1;
  this._userNode = node;
  this._curElement = null;
  this._elements = [];
  this._requestAnimId = 0;
  this._preloadCount = 0;
  this._isGlobal = false;
  this._loop = true;
  this._styles = [];
  this._defaultCanvasSize = {
    width: 100,
    height: 100
  };
  this._events = { // keep track of user-events for unregistering later
    'mousemove': null,
    'mousedown': null,
    'mouseup': null,
    'dragend': null,
    'dragover': null,
    'click': null,
    'mouseover': null,
    'mouseout': null,
    'keydown': null,
    'keyup': null,
    'keypress': null,
    'touchstart': null,
    'touchmove': null,
    'touchend': null,
    'resize': null,
    'blur': null
  };

  this._events.wheel = null;
  this._loadingScreenId = 'p5_loading';

  if (window.DeviceOrientationEvent) {
    this._events.deviceorientation = null;
  }
  if (window.DeviceMotionEvent && !window._isNodeWebkit) {
    this._events.devicemotion = null;
  }

  this._start = function () {
    // Find node if id given
    if (this._userNode) {
      if (typeof this._userNode === 'string') {
        this._userNode = document.getElementById(this._userNode);
      }
    }

    var userPreload = this.preload || window.preload; // look for "preload"
    if (userPreload) {

      // Setup loading screen
      // Set loading scfeen into dom if not present
      // Otherwise displays and removes user provided loading screen
      var loadingScreen = document.getElementById(this._loadingScreenId);
      if(!loadingScreen){
        loadingScreen = document.createElement('div');
        loadingScreen.innerHTML = 'Loading...';
        loadingScreen.style.position = 'absolute';
        loadingScreen.id = this._loadingScreenId;
        var node = this._userNode || document.body;
        node.appendChild(loadingScreen);
      }
      // var methods = this._preloadMethods;
      for (var method in this._preloadMethods){
        // default to p5 if no object defined
        this._preloadMethods[method] = this._preloadMethods[method] || p5;
        var obj = this._preloadMethods[method];
        //it's p5, check if it's global or instance
        if (obj === p5.prototype || obj === p5){
          obj = this._isGlobal ? window : this;
        }
        this._registeredPreloadMethods[method] = obj[method];
        obj[method] = this._wrapPreload(obj, method);
      }

      userPreload();
      this._runIfPreloadsAreDone();
    } else {
      this._setup();
      this._runFrames();
      this._draw();
    }
  }.bind(this);

  this._runIfPreloadsAreDone = function(){
    var context = this._isGlobal ? window : this;
    if (context._preloadCount === 0) {
      var loadingScreen = document.getElementById(context._loadingScreenId);
      if (loadingScreen) {
        loadingScreen.parentNode.removeChild(loadingScreen);
      }
      context._setup();
      context._runFrames();
      context._draw();
    }
  };

  this._decrementPreload = function(){
    var context = this._isGlobal ? window : this;
    if(typeof context.preload === 'function'){
      context._setProperty('_preloadCount', context._preloadCount - 1);
      context._runIfPreloadsAreDone();
    }
  };

  this._wrapPreload = function(obj, fnName){
    return function(){
      //increment counter
      this._incrementPreload();
      //call original function
      var args = new Array(arguments.length);
      for (var i = 0; i < args.length; ++i) {
        args[i] = arguments[i];
      }
      // args.push(this._decrementPreload.bind(this));
      return this._registeredPreloadMethods[fnName].apply(obj, args);
    }.bind(this);
  };

  this._incrementPreload = function(){
    var context = this._isGlobal ? window : this;
    context._setProperty('_preloadCount', context._preloadCount + 1);
  };

  this._setup = function() {

    // Always create a default canvas.
    // Later on if the user calls createCanvas, this default one
    // will be replaced
    this.createCanvas(
      this._defaultCanvasSize.width,
      this._defaultCanvasSize.height,
      'p2d',
      true
    );

    // return preload functions to their normal vals if switched by preload
    var context = this._isGlobal ? window : this;
    if (typeof context.preload === 'function') {
      for (var f in this._preloadMethods) {
        context[f] = this._preloadMethods[f][f];
        if (context[f] && this) {
          context[f] = context[f].bind(this);
        }
      }
    }

    // Short-circuit on this, in case someone used the library in "global"
    // mode earlier
    if (typeof context.setup === 'function') {
      context.setup();
    }

    // unhide any hidden canvases that were created
    var canvases = document.getElementsByTagName('canvas');
    for (var i = 0; i < canvases.length; i++) {
      var k = canvases[i];
      if (k.dataset.hidden === 'true') {
        k.style.visibility = '';
        delete(k.dataset.hidden);
      }
    }
    this._setupDone = true;

  }.bind(this);

  this._draw = function () {
    var now = window.performance.now();
    var time_since_last = now - this._lastFrameTime;
    var target_time_between_frames = 1000 / this._targetFrameRate;

    // only draw if we really need to; don't overextend the browser.
    // draw if we're within 5ms of when our next frame should paint
    // (this will prevent us from giving up opportunities to draw
    // again when it's really about time for us to do so). fixes an
    // issue where the frameRate is too low if our refresh loop isn't
    // in sync with the browser. note that we have to draw once even
    // if looping is off, so we bypass the time delay if that
    // is the case.
    var epsilon = 5;
    if (!this._loop ||
        time_since_last >= target_time_between_frames - epsilon) {

      //mandatory update values(matrixs and stack)

      this._setProperty('frameCount', this.frameCount + 1);
      this.redraw();
      this._frameRate = 1000.0/(now - this._lastFrameTime);
      this._lastFrameTime = now;

      // If the user is actually using mouse module, then update
      // coordinates, otherwise skip. We can test this by simply
      // checking if any of the mouse functions are available or not.
      // NOTE : This reflects only in complete build or modular build.
      if(typeof this._updateMouseCoords !== 'undefined') {
        this._updateMouseCoords();
      }
    }

    // get notified the next time the browser gives us
    // an opportunity to draw.
    if (this._loop) {
      this._requestAnimId = window.requestAnimationFrame(this._draw);
    }
  }.bind(this);

  this._runFrames = function() {
    if (this._updateInterval) {
      clearInterval(this._updateInterval);
    }
  }.bind(this);

  this._setProperty = function(prop, value) {
    this[prop] = value;
    if (this._isGlobal) {
      window[prop] = value;
    }
  }.bind(this);

  /**
   * Removes the entire p5 sketch. This will remove the canvas and any
   * elements created by p5.js. It will also stop the draw loop and unbind
   * any properties or methods from the window global scope. It will
   * leave a variable p5 in case you wanted to create a new p5 sketch.
   * If you like, you can set p5 = null to erase it.
   * @method remove
   * @example
   * <div class='norender'><code>
   * function draw() {
   *   ellipse(50, 50, 10, 10);
   * }
   *
   * function mousePressed() {
   *   remove(); // remove whole sketch on mouse press
   * }
   * </code></div>
   *
   * @alt
   * nothing displayed
   *
   */
  this.remove = function() {
    if (this._curElement) {

      // stop draw
      this._loop = false;
      if (this._requestAnimId) {
        window.cancelAnimationFrame(this._requestAnimId);
      }

      // unregister events sketch-wide
      for (var ev in this._events) {
        window.removeEventListener(ev, this._events[ev]);
      }

      // remove DOM elements created by p5, and listeners
      for (var i=0; i<this._elements.length; i++) {
        var e = this._elements[i];
        if (e.elt.parentNode) {
          e.elt.parentNode.removeChild(e.elt);
        }
        for (var elt_ev in e._events) {
          e.elt.removeEventListener(elt_ev, e._events[elt_ev]);
        }
      }

      // call any registered remove functions
      var self = this;
      this._registeredMethods.remove.forEach(function (f) {
        if (typeof(f) !== 'undefined') {
          f.call(self);
        }
      });

      // remove window bound properties and methods
      if (this._isGlobal) {
        for (var p in p5.prototype) {
          try {
            delete window[p];
          } catch (x) {
            window[p] = undefined;
          }
        }
        for (var p2 in this) {
          if (this.hasOwnProperty(p2)) {
            try {
              delete window[p2];
            } catch (x) {
              window[p2] = undefined;
            }
          }
        }
      }
    }
    // window.p5 = undefined;
  }.bind(this);

  // call any registered init functions
  this._registeredMethods.init.forEach(function (f) {
    if (typeof(f) !== 'undefined') {
      f.call(this);
    }
  }, this);

  var friendlyBindGlobal = this._createFriendlyGlobalFunctionBinder();

  // If the user has created a global setup or draw function,
  // assume "global" mode and make everything global (i.e. on the window)
  if (!sketch) {
    this._isGlobal = true;
    p5.instance = this;
    // Loop through methods on the prototype and attach them to the window
    for (var p in p5.prototype) {
      if(typeof p5.prototype[p] === 'function') {
        var ev = p.substring(2);
        if (!this._events.hasOwnProperty(ev)) {
          if (Math.hasOwnProperty(p) && (Math[p] === p5.prototype[p])) {
            // Multiple p5 methods are just native Math functions. These can be
            // called without any binding.
            friendlyBindGlobal(p, p5.prototype[p]);
          } else {
            friendlyBindGlobal(p, p5.prototype[p].bind(this));
          }
        }
      } else {
        friendlyBindGlobal(p, p5.prototype[p]);
      }
    }
    // Attach its properties to the window
    for (var p2 in this) {
      if (this.hasOwnProperty(p2)) {
        friendlyBindGlobal(p2, this[p2]);
      }
    }

  } else {
    // Else, the user has passed in a sketch closure that may set
    // user-provided 'setup', 'draw', etc. properties on this instance of p5
    sketch(this);
  }

  // Bind events to window (not using container div bc key events don't work)

  for (var e in this._events) {
    var f = this['_on'+e];
    if (f) {
      var m = f.bind(this);
      window.addEventListener(e, m, {passive: false});
      this._events[e] = m;
    }
  }

  var focusHandler = function() {
    this._setProperty('focused', true);
  }.bind(this);
  var blurHandler = function() {
    this._setProperty('focused', false);
  }.bind(this);
  window.addEventListener('focus', focusHandler);
  window.addEventListener('blur', blurHandler);
  this.registerMethod('remove', function() {
    window.removeEventListener('focus', focusHandler);
    window.removeEventListener('blur', blurHandler);
  });

  if (sync) {
    this._start();
  } else {
    if (document.readyState === 'complete') {
      this._start();
    } else {
      window.addEventListener('load', this._start.bind(this), false);
    }
  }
};

// This is a pointer to our global mode p5 instance, if we're in
// global mode.
p5.instance = null;

// Allows for the friendly error system to be turned off when creating a sketch,
// which can give a significant boost to performance when needed.
p5.disableFriendlyErrors = false;

// attach constants to p5 prototype
for (var k in constants) {
  p5.prototype[k] = constants[k];
}

// functions that cause preload to wait
// more can be added by using registerPreloadMethod(func)
p5.prototype._preloadMethods = {
  loadJSON: p5.prototype,
  loadImage: p5.prototype,
  loadStrings: p5.prototype,
  loadXML: p5.prototype,
  loadShape: p5.prototype,
  loadTable: p5.prototype,
  loadFont: p5.prototype,
  loadModel: p5.prototype
};

p5.prototype._registeredMethods = { init: [], pre: [], post: [], remove: [] };

p5.prototype._registeredPreloadMethods = {};

p5.prototype.registerPreloadMethod = function(fnString, obj) {
  // obj = obj || p5.prototype;
  if (!p5.prototype._preloadMethods.hasOwnProperty(fnString)) {
    p5.prototype._preloadMethods[fnString] = obj;
  }
};

p5.prototype.registerMethod = function(name, m) {
  if (!p5.prototype._registeredMethods.hasOwnProperty(name)) {
    p5.prototype._registeredMethods[name] = [];
  }
  p5.prototype._registeredMethods[name].push(m);
};

p5.prototype._createFriendlyGlobalFunctionBinder = function(options) {
  options = options || {};

  var globalObject = options.globalObject || window;
  var log = options.log || console.log.bind(console);
  var propsToForciblyOverwrite = {
    // p5.print actually always overwrites an existing global function,
    // albeit one that is very unlikely to be used:
    //
    //   https://developer.mozilla.org/en-US/docs/Web/API/Window/print
    'print': true
  };

  return function(prop, value) {
    if (!p5.disableFriendlyErrors &&
        typeof(IS_MINIFIED) === 'undefined' &&
        typeof(value) === 'function' &&
        !(prop in p5.prototype._preloadMethods)) {
      try {
        // Because p5 has so many common function names, it's likely
        // that users may accidentally overwrite global p5 functions with
        // their own variables. Let's allow this but log a warning to
        // help users who may be doing this unintentionally.
        //
        // For more information, see:
        //
        //   https://github.com/processing/p5.js/issues/1317

        if (prop in globalObject && !(prop in propsToForciblyOverwrite)) {
          throw new Error('global "' + prop + '" already exists');
        }

        // It's possible that this might throw an error because there
        // are a lot of edge-cases in which `Object.defineProperty` might
        // not succeed; since this functionality is only intended to
        // help beginners anyways, we'll just catch such an exception
        // if it occurs, and fall back to legacy behavior.
        Object.defineProperty(globalObject, prop, {
          configurable: true,
          enumerable: true,
          get: function() {
            return value;
          },
          set: function(newValue) {
            Object.defineProperty(globalObject, prop, {
              configurable: true,
              enumerable: true,
              value: newValue,
              writable: true
            });
            log(
              'You just changed the value of "' + prop + '", which was ' +
              'a p5 function. This could cause problems later if you\'re ' +
              'not careful.'
            );
          }
        });
      } catch (e) {
        log(
          'p5 had problems creating the global function "' + prop + '", ' +
          'possibly because your code is already using that name as ' +
          'a variable. You may want to rename your variable to something ' +
          'else.'
        );
        globalObject[prop] = value;
      }
    } else {
      globalObject[prop] = value;
    }
  };
};


module.exports = p5;

},{"./constants":53,"./shim":64}],55:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule Curves
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('./core');

_dereq_('./error_helpers');

var bezierDetail = 20;
var curveDetail = 20;

/**
 * Draws a cubic Bezier curve on the screen. These curves are defined by a
 * series of anchor and control points. The first two parameters specify
 * the first anchor point and the last two parameters specify the other
 * anchor point, which become the first and last points on the curve. The
 * middle parameters specify the two control points which define the shape
 * of the curve. Approximately speaking, control points "pull" the curve
 * towards them.<br /><br />Bezier curves were developed by French
 * automotive engineer Pierre Bezier, and are commonly used in computer
 * graphics to define gently sloping curves. See also curve().
 *
 * @method bezier
 * @param  {Number} x1 x-coordinate for the first anchor point
 * @param  {Number} y1 y-coordinate for the first anchor point
 * @param  {Number} x2 x-coordinate for the first control point
 * @param  {Number} y2 y-coordinate for the first control point
 * @param  {Number} x3 x-coordinate for the second control point
 * @param  {Number} y3 y-coordinate for the second control point
 * @param  {Number} x4 x-coordinate for the second anchor point
 * @param  {Number} y4 y-coordinate for the second anchor point
 * @return {p5}        the p5 object
 * @example
 * <div>
 * <code>
 * noFill();
 * stroke(255, 102, 0);
 * line(85, 20, 10, 10);
 * line(90, 90, 15, 80);
 * stroke(0, 0, 0);
 * bezier(85, 20, 10, 10, 90, 90, 15, 80);
 * </code>
 * </div>
 * @alt
 * stretched black s-shape in center with orange lines extending from end points.
 * stretched black s-shape with 10 5x5 white ellipses along the shape.
 * stretched black s-shape with 7 5x5 ellipses and orange lines along the shape.
 * stretched black s-shape with 17 small orange lines extending from under shape.
 * horseshoe shape with orange ends facing left and black curved center.
 * horseshoe shape with orange ends facing left and black curved center.
 * Line shaped like right-facing arrow,points move with mouse-x and warp shape.
 * horizontal line that hooks downward on the right and 13 5x5 ellipses along it.
 * right curving line mid-right of canvas with 7 short lines radiating from it.
 */
/**
 * @method bezier
 * @param  {Number} z1 z-coordinate for the first anchor point
 * @param  {Number} z2 z-coordinate for the first control point
 * @param  {Number} z3 z-coordinate for the first anchor point
 * @param  {Number} z4 z-coordinate for the first control point
 * @chainable
 * @example
 * <div>
 * <code>
 *background(0, 0, 0);
 *noFill();
 *stroke(255);
 *bezier(250,250,0, 100,100,0, 100,0,0, 0,100,0);
 * </code>
 * </div>
*/
p5.prototype.bezier = function() {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if (!this._renderer._doStroke && !this._renderer._doFill) {
    return this;
  }
  if (this._renderer.isP3D){
    args.push(bezierDetail);//adding value of bezier detail to the args array
    this._renderer.bezier(args);
  } else{
    this._renderer.bezier(args[0],args[1],
      args[2],args[3],
      args[4],args[5],
      args[6],args[7]);
  }

  return this;
};

/**
 * Sets the resolution at which Beziers display.
 *
 * The default value is 20.
 *
 * @param {Number} detail resolution of the curves
 * @chainable
 * @example
 * <div>
 * <code>
 * background(204);
 * bezierDetail(50);
 * bezier(85, 20, 10, 10, 90, 90, 15, 80);
 * </code>
 * </div>
 *
 * @alt
 * stretched black s-shape with 7 5x5 ellipses and orange lines along the shape.
 *
 */
p5.prototype.bezierDetail = function(d) {
  bezierDetail = d;
  return this;
};

/**
 * Evaluates the Bezier at position t for points a, b, c, d.
 * The parameters a and d are the first and last points
 * on the curve, and b and c are the control points.
 * The final parameter t varies between 0 and 1.
 * This can be done once with the x coordinates and a second time
 * with the y coordinates to get the location of a bezier curve at t.
 *
 * @method bezierPoint
 * @param {Number} a coordinate of first point on the curve
 * @param {Number} b coordinate of first control point
 * @param {Number} c coordinate of second control point
 * @param {Number} d coordinate of second point on the curve
 * @param {Number} t value between 0 and 1
 * @return {Number} the value of the Bezier at position t
 * @example
 * <div>
 * <code>
 * noFill();
 * x1 = 85, x2 = 10, x3 = 90, x4 = 15;
 * y1 = 20, y2 = 10, y3 = 90, y4 = 80;
 * bezier(x1, y1, x2, y2, x3, y3, x4, y4);
 * fill(255);
 * steps = 10;
 * for (i = 0; i <= steps; i++) {
 *   t = i / steps;
 *   x = bezierPoint(x1, x2, x3, x4, t);
 *   y = bezierPoint(y1, y2, y3, y4, t);
 *   ellipse(x, y, 5, 5);
 * }
 * </code>
 * </div>
 *
 * @alt
 * stretched black s-shape with 17 small orange lines extending from under shape.
 *
 */
p5.prototype.bezierPoint = function(a, b, c, d, t) {
  var adjustedT = 1-t;
  return Math.pow(adjustedT,3)*a +
   3*(Math.pow(adjustedT,2))*t*b +
   3*adjustedT*Math.pow(t,2)*c +
   Math.pow(t,3)*d;
};

/**
 * Evaluates the tangent to the Bezier at position t for points a, b, c, d.
 * The parameters a and d are the first and last points
 * on the curve, and b and c are the control points.
 * The final parameter t varies between 0 and 1.
 *
 * @method bezierTangent
 * @param {Number} a coordinate of first point on the curve
 * @param {Number} b coordinate of first control point
 * @param {Number} c coordinate of second control point
 * @param {Number} d coordinate of second point on the curve
 * @param {Number} t value between 0 and 1
 * @return {Number} the tangent at position t
 * @example
 * <div>
 * <code>
 * noFill();
 * bezier(85, 20, 10, 10, 90, 90, 15, 80);
 * steps = 6;
 * fill(255);
 * for (i = 0; i <= steps; i++) {
 *   t = i / steps;
 *   // Get the location of the point
 *   x = bezierPoint(85, 10, 90, 15, t);
 *   y = bezierPoint(20, 10, 90, 80, t);
 *   // Get the tangent points
 *   tx = bezierTangent(85, 10, 90, 15, t);
 *   ty = bezierTangent(20, 10, 90, 80, t);
 *   // Calculate an angle from the tangent points
 *   a = atan2(ty, tx);
 *   a += PI;
 *   stroke(255, 102, 0);
 *   line(x, y, cos(a)*30 + x, sin(a)*30 + y);
 *   // The following line of code makes a line
 *   // inverse of the above line
 *   //line(x, y, cos(a)*-30 + x, sin(a)*-30 + y);
 *   stroke(0);
 *   ellipse(x, y, 5, 5);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * bezier(85, 20, 10, 10, 90, 90, 15, 80);
 * stroke(255, 102, 0);
 * steps = 16;
 * for (i = 0; i <= steps; i++) {
 *   t = i / steps;
 *   x = bezierPoint(85, 10, 90, 15, t);
 *   y = bezierPoint(20, 10, 90, 80, t);
 *   tx = bezierTangent(85, 10, 90, 15, t);
 *   ty = bezierTangent(20, 10, 90, 80, t);
 *   a = atan2(ty, tx);
 *   a -= HALF_PI;
 *   line(x, y, cos(a)*8 + x, sin(a)*8 + y);
 * }
 * </code>
 * </div>
 *
 * @alt
 * s-shaped line with 17 short orange lines extending from underside of shape
 *
 */
p5.prototype.bezierTangent = function(a, b, c, d, t) {
  var adjustedT = 1-t;
  return 3*d*Math.pow(t,2) -
   3*c*Math.pow(t,2) +
   6*c*adjustedT*t -
   6*b*adjustedT*t +
   3*b*Math.pow(adjustedT,2) -
   3*a*Math.pow(adjustedT,2);
};

/**
 * Draws a curved line on the screen between two points, given as the
 * middle four parameters. The first two parameters are a control point, as
 * if the curve came from this point even though it's not drawn. The last
 * two parameters similarly describe the other control point. <br /><br />
 * Longer curves can be created by putting a series of curve() functions
 * together or using curveVertex(). An additional function called
 * curveTightness() provides control for the visual quality of the curve.
 * The curve() function is an implementation of Catmull-Rom splines.
 *
 * @method curve
 * @param  {Number} x1 x-coordinate for the beginning control point
 * @param  {Number} y1 y-coordinate for the beginning control point
 * @param  {Number} x2 x-coordinate for the first point
 * @param  {Number} y2 y-coordinate for the first point
 * @param  {Number} x3 x-coordinate for the second point
 * @param  {Number} y3 y-coordinate for the second point
 * @param  {Number} x4 x-coordinate for the ending control point
 * @param  {Number} y4 y-coordinate for the ending control point
 * @return {p5}        the p5 object
 * @example
 * <div>
 * <code>
 * noFill();
 * stroke(255, 102, 0);
 * curve(5, 26, 5, 26, 73, 24, 73, 61);
 * stroke(0);
 * curve(5, 26, 73, 24, 73, 61, 15, 65);
 * stroke(255, 102, 0);
 * curve(73, 24, 73, 61, 15, 65, 15, 65);
 * </code>
 * </div>
 * <div>
 * <code>
 * // Define the curve points as JavaScript objects
 * p1 = {x: 5, y: 26}, p2 = {x: 73, y: 24}
 * p3 = {x: 73, y: 61}, p4 = {x: 15, y: 65}
 * noFill();
 * stroke(255, 102, 0);
 * curve(p1.x, p1.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y)
 * stroke(0);
 * curve(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y)
 * stroke(255, 102, 0);
 * curve(p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, p4.x, p4.y)
 * </code>
 * </div>
 *
 * @alt
 * horseshoe shape with orange ends facing left and black curved center.
 * horseshoe shape with orange ends facing left and black curved center.
 *
 */
/**
 * @method curve
 * @param  {Number} z1 z-coordinate for the beginning control point
 * @param  {Number} z2 z-coordinate for the first point
 * @param  {Number} z3 z-coordinate for the second point
 * @param  {Number} z4 z-coordinate for the ending control point
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 * stroke(255, 102, 0);
 * curve(5,26,0, 5,26,0, 73,24,0, 73,61,0);
 * stroke(0);
 * curve(5,26,0, 73,24,0, 73,61,0, 15,65,0);
 * stroke(255, 102, 0);
 * curve(73,24,0, 73,61,0, 15,65,0, 15,65,0);
 * </code>
 * </div>
 *
 * @alt
 * curving black and orange lines.
 */
p5.prototype.curve = function() {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if (!this._renderer._doStroke) {
    return this;
  }
  if (this._renderer.isP3D){
    args.push(curveDetail);
    this._renderer.curve(args);
  } else{
    this._renderer.curve(args[0],args[1],
      args[2],args[3],
      args[4],args[5],
      args[6],args[7]);
  }
  return this;
};

/**
 * Sets the resolution at which curves display.
 *
 * The default value is 20.
 *
 * @param {Number} resolution of the curves
 * @chainable
 * @example
 * <div>
 * <code>
 * background(204);
 * curveDetail(20);
 * curve(5, 26, 5, 26, 73, 24, 73, 61);
 * </code>
 * </div>
 *
 * @alt
 * white arch shape in top-mid canvas.
 *
 */
p5.prototype.curveDetail = function(d) {
  curveDetail = d;
  return this;
};

/**
 * Modifies the quality of forms created with curve() and curveVertex().
 * The parameter tightness determines how the curve fits to the vertex
 * points. The value 0.0 is the default value for tightness (this value
 * defines the curves to be Catmull-Rom splines) and the value 1.0 connects
 * all the points with straight lines. Values within the range -5.0 and 5.0
 * will deform the curves but will leave them recognizable and as values
 * increase in magnitude, they will continue to deform.
 *
 * @method curveTightness
 * @param {Number} amount of deformation from the original vertices
 * @chainable
 * @example
 * <div>
 * <code>
 * // Move the mouse left and right to see the curve change
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   noFill();
 * }
 *
 * function draw() {
 *   background(204);
 *   var t = map(mouseX, 0, width, -5, 5);
 *   curveTightness(t);
 *   beginShape();
 *   curveVertex(10, 26);
 *   curveVertex(10, 26);
 *   curveVertex(83, 24);
 *   curveVertex(83, 61);
 *   curveVertex(25, 65);
 *   curveVertex(25, 65);
 *   endShape();
 * }
 * </code>
 * </div>
 *
 * @alt
 * Line shaped like right-facing arrow,points move with mouse-x and warp shape.
 */
p5.prototype.curveTightness = function (t) {
  this._renderer._curveTightness = t;
};

/**
 * Evaluates the curve at position t for points a, b, c, d.
 * The parameter t varies between 0 and 1, a and d are points
 * on the curve, and b and c are the control points.
 * This can be done once with the x coordinates and a second time
 * with the y coordinates to get the location of a curve at t.
 *
 * @method curvePoint
 * @param {Number} a coordinate of first point on the curve
 * @param {Number} b coordinate of first control point
 * @param {Number} c coordinate of second control point
 * @param {Number} d coordinate of second point on the curve
 * @param {Number} t value between 0 and 1
 * @return {Number} bezier value at position t
 * @example
 * <div>
 * <code>
 * noFill();
 * curve(5, 26, 5, 26, 73, 24, 73, 61);
 * curve(5, 26, 73, 24, 73, 61, 15, 65);
 * fill(255);
 * ellipseMode(CENTER);
 * steps = 6;
 * for (i = 0; i <= steps; i++) {
 *   t = i / steps;
 *   x = curvePoint(5, 5, 73, 73, t);
 *   y = curvePoint(26, 26, 24, 61, t);
 *   ellipse(x, y, 5, 5);
 *   x = curvePoint(5, 73, 73, 15, t);
 *   y = curvePoint(26, 24, 61, 65, t);
 *   ellipse(x, y, 5, 5);
 * }
 * </code>
 * </div>
 *
 *line hooking down to right-bottom with 13 5x5 white ellipse points
 */
p5.prototype.curvePoint = function(a, b, c, d, t) {
  var t3 = t*t*t,
    t2 = t*t,
    f1 = -0.5 * t3 + t2 - 0.5 * t,
    f2 = 1.5 * t3 - 2.5 * t2 + 1.0,
    f3 = -1.5 * t3 + 2.0 * t2 + 0.5 * t,
    f4 = 0.5 * t3 - 0.5 * t2;
  return a*f1 + b*f2 + c*f3 + d*f4;
};

/**
 * Evaluates the tangent to the curve at position t for points a, b, c, d.
 * The parameter t varies between 0 and 1, a and d are points on the curve,
 * and b and c are the control points.
 *
 * @method curveTangent
 * @param {Number} a coordinate of first point on the curve
 * @param {Number} b coordinate of first control point
 * @param {Number} c coordinate of second control point
 * @param {Number} d coordinate of second point on the curve
 * @param {Number} t value between 0 and 1
 * @return {Number} the tangent at position t
 * @example
 * <div>
 * <code>
 * noFill();
 * curve(5, 26, 73, 24, 73, 61, 15, 65);
 * steps = 6;
 * for (i = 0; i <= steps; i++) {
 *   t = i / steps;
 *   x = curvePoint(5, 73, 73, 15, t);
 *   y = curvePoint(26, 24, 61, 65, t);
 *   //ellipse(x, y, 5, 5);
 *   tx = curveTangent(5, 73, 73, 15, t);
 *   ty = curveTangent(26, 24, 61, 65, t);
 *   a = atan2(ty, tx);
 *   a -= PI/2.0;
 *   line(x, y, cos(a)*8 + x, sin(a)*8 + y);
 * }
 * </code>
 * </div>
 *
 * @alt
 *right curving line mid-right of canvas with 7 short lines radiating from it.
 */
p5.prototype.curveTangent = function(a, b,c, d, t) {
  var t2 = t*t,
    f1 = (-3*t2)/2 + 2*t - 0.5,
    f2 = (9*t2)/2 - 5*t,
    f3 = (-9*t2)/2 + 4*t + 0.5,
    f4 = (3*t2)/2 - t;
  return a*f1 + b*f2 + c*f3 + d*f4;
};

module.exports = p5;

},{"./core":54,"./error_helpers":57}],56:[function(_dereq_,module,exports){
/**
 * @module Environment
 * @submodule Environment
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('./core');
var C = _dereq_('./constants');

var standardCursors = [C.ARROW, C.CROSS, C.HAND, C.MOVE, C.TEXT, C.WAIT];

p5.prototype._frameRate = 0;
p5.prototype._lastFrameTime = window.performance.now();
p5.prototype._targetFrameRate = 60;

var _windowPrint = window.print;


if (window.console && console.log) {
  /**
   * The print() function writes to the console area of your browser.
   * This function is often helpful for looking at the data a program is
   * producing. This function creates a new line of text for each call to
   * the function. Individual elements can be
   * separated with quotes ("") and joined with the addition operator (+).
   * <br><br>
   * While print() is similar to console.log(), it does not directly map to
   * it in order to simulate easier to understand behavior than
   * console.log(). Due to this, it is slower. For fastest results, use
   * console.log().
   *
   * @method print
   * @param {Any} contents any combination of Number, String, Object, Boolean,
   *                       Array to print
   * @example
   * <div><code class='norender'>
   * var x = 10;
   * print("The value of x is " + x);
   * // prints "The value of x is 10"
   * </code></div>
   * @alt
   * default grey canvas
   */
  // Converts passed args into a string and then parses that string to
  // simulate synchronous behavior. This is a hack and is gross.
  // Since this will not work on all objects, particularly circular
  // structures, simply console.log() on error.
  p5.prototype.print = function(args) {
    try {
      if (arguments.length === 0) {
        _windowPrint();
      }
      else if (arguments.length > 1) {
        console.log.apply(console, arguments);
      } else {
        var newArgs = JSON.parse(JSON.stringify(args));
        if (JSON.stringify(newArgs)==='{}'){
          console.log(args);
        } else {
          console.log(newArgs);
        }
      }
    } catch(err) {
      console.log(args);
    }
  };
} else {
  p5.prototype.print = function() {};
}


/**
 * The system variable frameCount contains the number of frames that have
 * been displayed since the program started. Inside setup() the value is 0,
 * after the first iteration of draw it is 1, etc.
 *
 * @property {Number} frameCount
 * @readOnly
 * @example
 *   <div><code>
 *     function setup() {
 *       frameRate(30);
 *       textSize(20);
 *       textSize(30);
 *       textAlign(CENTER);
 *     }
 *
 *     function draw() {
 *       background(200);
 *       text(frameCount, width/2, height/2);
 *     }
 *   </code></div>
 *
 * @alt
 * numbers rapidly counting upward with frame count set to 30.
 *
 */
p5.prototype.frameCount = 0;

/**
 * Confirms if the window a p5.js program is in is "focused," meaning that
 * the sketch will accept mouse or keyboard input. This variable is
 * "true" if the window is focused and "false" if not.
 *
 * @property {Boolean} focused
 * @readOnly
 * @example
 * <div><code>
 * // To demonstrate, put two windows side by side.
 * // Click on the window that the p5 sketch isn't in!
 * function draw() {
 *   background(200);
 *   noStroke();
 *   fill(0, 200, 0);
 *   ellipse(25, 25, 50, 50);
 *
 *   if (!focused) {  // or "if (focused === false)"
 *     stroke(200,0,0);
 *     line(0, 0, 100, 100);
 *     line(100, 0, 0, 100);
 *   }
 * }
 * </code></div>
 *
 * @alt
 * green 50x50 ellipse at top left. Red X covers canvas when page focus changes
 *
 */
p5.prototype.focused = (document.hasFocus());

/**
 * Sets the cursor to a predefined symbol or an image, or makes it visible
 * if already hidden. If you are trying to set an image as the cursor, the
 * recommended size is 16x16 or 32x32 pixels. It is not possible to load an
 * image as the cursor if you are exporting your program for the Web, and not
 * all MODES work with all browsers. The values for parameters x and y must
 * be less than the dimensions of the image.
 *
 * @method cursor
 * @param {String|Constant} type either ARROW, CROSS, HAND, MOVE, TEXT, or
 *                               WAIT, or path for image
 * @param {Number}          [x]  the horizontal active spot of the cursor
 * @param {Number}          [y]  the vertical active spot of the cursor
 * @example
 * <div><code>
 * // Move the mouse left and right across the image
 * // to see the cursor change from a cross to a hand
 * function draw() {
 *   line(width/2, 0, width/2, height);
 *   if (mouseX < 50) {
 *     cursor(CROSS);
 *   } else {
 *     cursor(HAND);
 *   }
 * }
 * </code></div>
 *
 * @alt
 * horizontal line divides canvas. cursor on left is a cross, right is hand.
 *
 */
p5.prototype.cursor = function(type, x, y) {
  var cursor = 'auto';
  var canvas = this._curElement.elt;
  if (standardCursors.indexOf(type) > -1) {
    // Standard css cursor
    cursor = type;
  } else if (typeof type === 'string') {
    var coords = '';
    if (x && y && (typeof x === 'number' && typeof y === 'number')) {
      // Note that x and y values must be unit-less positive integers < 32
      // https://developer.mozilla.org/en-US/docs/Web/CSS/cursor
      coords = x + ' ' + y;
    }
    if ((type.substring(0, 7) === 'http://') ||
        (type.substring(0, 8) === 'https://')) {
      // Image (absolute url)
      cursor = 'url(' + type + ') ' + coords + ', auto';
    } else if (/\.(cur|jpg|jpeg|gif|png|CUR|JPG|JPEG|GIF|PNG)$/.test(type)) {
      // Image file (relative path) - Separated for performance reasons
      cursor = 'url(' + type + ') ' + coords + ', auto';
    } else {
      // Any valid string for the css cursor property
      cursor = type;
    }
  }
  canvas.style.cursor = cursor;
};

/**
 * Specifies the number of frames to be displayed every second. For example,
 * the function call frameRate(30) will attempt to refresh 30 times a second.
 * If the processor is not fast enough to maintain the specified rate, the
 * frame rate will not be achieved. Setting the frame rate within setup() is
 * recommended. The default rate is 60 frames per second. This is the same as
 * setFrameRate(val).
 * <br><br>
 * Calling frameRate() with no arguments returns the current framerate. The
 * draw function must run at least once before it will return a value. This
 * is the same as getFrameRate().
 * <br><br>
 * Calling frameRate() with arguments that are not of the type numbers
 * or are non positive also returns current framerate.
 *
 * @method frameRate
 * @param  {Number} fps number of frames to be displayed every second
 * @chainable
 */
/**
 * @method frameRate
 * @return {Number}       current frameRate
 * @example
 *
 * <div><code>
 * var rectX = 0;
 * var fr = 30; //starting FPS
 * var clr;
 *
 * function setup() {
 *   background(200);
 *   frameRate(fr); // Attempt to refresh at starting FPS
 *   clr = color(255,0,0);
 * }
 *
 * function draw() {
 *   background(200);
 *   rectX = rectX += 1; // Move Rectangle
 *
 *   if (rectX >= width) { // If you go off screen.
 *     if (fr == 30) {
 *       clr = color(0,0,255);
 *       fr = 10;
 *       frameRate(fr); // make frameRate 10 FPS
 *     } else {
 *       clr = color(255,0,0);
 *       fr = 30;
 *       frameRate(fr); // make frameRate 30 FPS
 *     }
 *     rectX = 0;
 *   }
 *   fill(clr);
 *   rect(rectX, 40, 20,20);
 * }
 * </div></code>
 *
 * @alt
 * blue rect moves left to right, followed by red rect moving faster. Loops.
 *
 */
p5.prototype.frameRate = function(fps) {
  if (typeof fps !== 'number' || fps < 0) {
    return this._frameRate;
  } else {
    this._setProperty('_targetFrameRate', fps);
    this._runFrames();
    return this;
  }
};
/**
 * Returns the current framerate.
 *
 * @return {Number} current frameRate
 */
p5.prototype.getFrameRate = function() {
  return this.frameRate();
};

/**
 * Specifies the number of frames to be displayed every second. For example,
 * the function call frameRate(30) will attempt to refresh 30 times a second.
 * If the processor is not fast enough to maintain the specified rate, the
 * frame rate will not be achieved. Setting the frame rate within setup() is
 * recommended. The default rate is 60 frames per second.
 *
 * Calling frameRate() with no arguments returns the current framerate.
 *
 * @param {Number} [fps] number of frames to be displayed every second
 */
p5.prototype.setFrameRate = function(fps) {
  return this.frameRate(fps);
};

/**
 * Hides the cursor from view.
 *
 * @method noCursor
 * @example
 * <div><code>
 * function setup() {
 *   noCursor();
 * }
 *
 * function draw() {
 *   background(200);
 *   ellipse(mouseX, mouseY, 10, 10);
 * }
 * </code></div>
 *
 *
 * @alt
 * cursor becomes 10x 10 white ellipse the moves with mouse x and y.
 *
 */
p5.prototype.noCursor = function() {
  this._curElement.elt.style.cursor = 'none';
};


/**
 * System variable that stores the width of the entire screen display. This
 * is used to run a full-screen program on any display size.
 *
 * @property {Number} displayWidth
 * @readOnly
 * @example
 * <div class="norender"><code>
 * createCanvas(displayWidth, displayHeight);
 * </code></div>
 *
 * @alt
 * cursor becomes 10x 10 white ellipse the moves with mouse x and y.
 *
 */
p5.prototype.displayWidth = screen.width;

/**
 * System variable that stores the height of the entire screen display. This
 * is used to run a full-screen program on any display size.
 *
 * @property {Number} displayHeight
 * @readOnly
 * @example
 * <div class="norender"><code>
 * createCanvas(displayWidth, displayHeight);
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.prototype.displayHeight = screen.height;

/**
 * System variable that stores the width of the inner window, it maps to
 * window.innerWidth.
 *
 * @property {Number} windowWidth
 * @readOnly
 * @example
 * <div class="norender"><code>
 * createCanvas(windowWidth, windowHeight);
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.prototype.windowWidth = getWindowWidth();
/**
 * System variable that stores the height of the inner window, it maps to
 * window.innerHeight.
 *
 * @property {Number} windowHeight
 * @readOnly
 * @example
 * <div class="norender"><code>
 * createCanvas(windowWidth, windowHeight);
 * </code></div>
*@alt
 * no display.
 *
*/
p5.prototype.windowHeight = getWindowHeight();

/**
 * The windowResized() function is called once every time the browser window
 * is resized. This is a good place to resize the canvas or do any other
 * adjustments to accommodate the new window size.
 *
 * @method windowResized
 * @example
 * <div class="norender"><code>
 * function setup() {
 *   createCanvas(windowWidth, windowHeight);
 * }
 *
 * function draw() {
 *  background(0, 100, 200);
 * }
 *
 * function windowResized() {
 *   resizeCanvas(windowWidth, windowHeight);
 * }
 * </code></div>
 * @alt
 * no display.
 */
p5.prototype._onresize = function(e){
  this._setProperty('windowWidth', getWindowWidth());
  this._setProperty('windowHeight', getWindowHeight());
  var context = this._isGlobal ? window : this;
  var executeDefault;
  if (typeof context.windowResized === 'function') {
    executeDefault = context.windowResized(e);
    if (executeDefault !== undefined && !executeDefault) {
      e.preventDefault();
    }
  }
};

function getWindowWidth() {
  return window.innerWidth ||
         document.documentElement && document.documentElement.clientWidth ||
         document.body && document.body.clientWidth ||
         0;
}

function getWindowHeight() {
  return window.innerHeight ||
         document.documentElement && document.documentElement.clientHeight ||
         document.body && document.body.clientHeight ||
         0;
}

/**
 * System variable that stores the width of the drawing canvas. This value
 * is set by the first parameter of the createCanvas() function.
 * For example, the function call createCanvas(320, 240) sets the width
 * variable to the value 320. The value of width defaults to 100 if
 * createCanvas() is not used in a program.
 *
 * @property {Number} width
 * @readOnly
 */
p5.prototype.width = 0;

/**
 * System variable that stores the height of the drawing canvas. This value
 * is set by the second parameter of the createCanvas() function. For
 * example, the function call createCanvas(320, 240) sets the height
 * variable to the value 240. The value of height defaults to 100 if
 * createCanvas() is not used in a program.
 *
 * @property {Number} height
 * @readOnly
 */
p5.prototype.height = 0;

/**
 * If argument is given, sets the sketch to fullscreen or not based on the
 * value of the argument. If no argument is given, returns the current
 * fullscreen state. Note that due to browser restrictions this can only
 * be called on user input, for example, on mouse press like the example
 * below.
 *
 * @method fullscreen
 * @param  {Boolean} [val] whether the sketch should be in fullscreen mode
 * or not
 * @return {Boolean} current fullscreen state
 * @example
 * <div>
 * <code>
 * // Clicking in the box toggles fullscreen on and off.
 * function setup() {
 *   background(200);
 * }
 * function mousePressed() {
 *   if (mouseX > 0 && mouseX < 100 && mouseY > 0 && mouseY < 100) {
 *     var fs = fullscreen();
 *     fullscreen(!fs);
 *   }
 * }
 * </code>
 * </div>
 *
 * @alt
 * no display.
 *
 */
p5.prototype.fullscreen = function(val) {
  // no arguments, return fullscreen or not
  if (typeof val === 'undefined') {
    return document.fullscreenElement ||
           document.webkitFullscreenElement ||
           document.mozFullScreenElement ||
           document.msFullscreenElement;
  } else { // otherwise set to fullscreen or not
    if (val) {
      launchFullscreen(document.documentElement);
    } else {
      exitFullscreen();
    }
  }
};

/**
 * Sets the pixel scaling for high pixel density displays. By default
 * pixel density is set to match display density, call pixelDensity(1)
 * to turn this off. Calling pixelDensity() with no arguments returns
 * the current pixel density of the sketch.
 *
 *
 * @method pixelDensity
 * @param  {Number} [val] whether or how much the sketch should scale
 * @returns {Number} current pixel density of the sketch
 * @example
 * <div>
 * <code>
 * function setup() {
 *   pixelDensity(1);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width/2, height/2, 50, 50);
 * }
 * </code>
 * </div>
 * <div>
 * <code>
 * function setup() {
 *   pixelDensity(3.0);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width/2, height/2, 50, 50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * fuzzy 50x50 white ellipse with black outline in center of canvas.
 * sharp 50x50 white ellipse with black outline in center of canvas.
 */
p5.prototype.pixelDensity = function(val) {
  if (typeof val === 'number') {
    this._pixelDensity = val;
  } else {
    return this._pixelDensity;
  }
  this.resizeCanvas(this.width, this.height, true);
};

/**
 * Returns the pixel density of the current display the sketch is running on.
 *
 * @method displayDensity
 * @returns {Number} current pixel density of the display
 * @example
 * <div>
 * <code>
 * function setup() {
 *   var density = displayDensity();
 *   pixelDensity(density);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width/2, height/2, 50, 50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 50x50 white ellipse with black outline in center of canvas.
 */
p5.prototype.displayDensity = function() {
  return window.devicePixelRatio;
};

function launchFullscreen(element) {
  var enabled = document.fullscreenEnabled ||
                document.webkitFullscreenEnabled ||
                document.mozFullScreenEnabled ||
                document.msFullscreenEnabled;
  if (!enabled) {
    throw new Error('Fullscreen not enabled in this browser.');
  }
  if(element.requestFullscreen) {
    element.requestFullscreen();
  } else if(element.mozRequestFullScreen) {
    element.mozRequestFullScreen();
  } else if(element.webkitRequestFullscreen) {
    element.webkitRequestFullscreen();
  } else if(element.msRequestFullscreen) {
    element.msRequestFullscreen();
  }
}

function exitFullscreen() {
  if(document.exitFullscreen) {
    document.exitFullscreen();
  } else if(document.mozCancelFullScreen) {
    document.mozCancelFullScreen();
  } else if(document.webkitExitFullscreen) {
    document.webkitExitFullscreen();
  } else if (document.msExitFullscreen) {
    document.msExitFullscreen();
  }
}


/**
 * Gets the current URL.
 * @method getURL
 * @return {String} url
 * @example
 * <div>
 * <code>
 * var url;
 * var x = 100;
 *
 * function setup() {
 *   fill(0);
 *   noStroke();
 *   url = getURL();
 * }
 *
 * function draw() {
 *   background(200);
 *   text(url, x, height/2);
 *   x--;
 * }
 * </code>
 * </div>
 *
 * @alt
 * current url (http://p5js.org/reference/#/p5/getURL) moves right to left.
 *
 */
p5.prototype.getURL = function() {
  return location.href;
};
/**
 * Gets the current URL path as an array.
 * @method getURLPath
 * @return {String[]} path components
 * @example
 * <div class='norender'><code>
 * function setup() {
 *   var urlPath = getURLPath();
 *   for (var i=0; i&lt;urlPath.length; i++) {
 *     text(urlPath[i], 10, i*20+20);
 *   }
 * }
 * </code></div>
 *
 * @alt
 *no display
 *
 */
p5.prototype.getURLPath = function() {
  return location.pathname.split('/').filter(function(v){return v!=='';});
};
/**
 * Gets the current URL params as an Object.
 * @method getURLParams
 * @return {Object} URL params
 * @example
 * <div class='norender'>
 * <code>
 * // Example: http://p5js.org?year=2014&month=May&day=15
 *
 * function setup() {
 *   var params = getURLParams();
 *   text(params.day, 10, 20);
 *   text(params.month, 10, 40);
 *   text(params.year, 10, 60);
 * }
 * </code>
 * </div>
 * @alt
 * no display.
 *
 */
p5.prototype.getURLParams = function() {
  var re = /[?&]([^&=]+)(?:[&=])([^&=]+)/gim;
  var m;
  var v={};
  while ((m = re.exec(location.search)) != null) {
    if (m.index === re.lastIndex) {
      re.lastIndex++;
    }
    v[m[1]]=m[2];
  }
  return v;
};

module.exports = p5;

},{"./constants":53,"./core":54}],57:[function(_dereq_,module,exports){
/**
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('./core');
var doFriendlyWelcome = false; // TEMP until we get it all working LM

// -- Borrowed from jQuery 1.11.3 --
var class2type = {};
var toString = class2type.toString;
var names = ['Boolean', 'Number', 'String', 'Function',
             'Array', 'Date', 'RegExp', 'Object', 'Error'];
for (var n=0; n<names.length; n++) {
  class2type[ '[object ' + names[n] + ']' ] = names[n].toLowerCase();
}
var getType = function( obj ) {
  if ( obj == null ) {
    return obj + '';
  }
  return typeof obj === 'object' || typeof obj === 'function' ?
    class2type[ toString.call(obj) ] || 'object' :
    typeof obj;
};

// -- End borrow --


/**
 * Prints out a fancy, colorful message to the console log
 *
 * @param  {String}               message the words to be said
 * @param  {String}               func    the name of the function to link
 * @param  {Number|String} color   CSS color string or error type
 *
 * @return console logs
 */
// Wrong number of params, undefined param, wrong type
var FILE_LOAD = 3;
// p5.js blue, p5.js orange, auto dark green; fallback p5.js darkened magenta
// See testColors below for all the color codes and names
var typeColors = ['#2D7BB6', '#EE9900', '#4DB200', '#C83C00'];
function report(message, func, color) {
  if(doFriendlyWelcome){
    friendlyWelcome();
    doFriendlyWelcome =false;
  }
  if ('undefined' === getType(color)) {
    color   = '#B40033'; // dark magenta
  } else if (getType(color) === 'number') { // Type to color
    color = typeColors[color];
  }
  // LM TEMP commenting this out until we get the whole system working
  // if (func.substring(0,4) === 'load'){
  //   console.log(
  //     '%c> p5.js says: '+message+'%c'+
  //     '[https://github.com/processing/p5.js/wiki/Local-server]',
  //     'background-color:' + color + ';color:#FFF;',
  //     'background-color:transparent;color:' + color +';',
  //     'background-color:' + color + ';color:#FFF;',
  //     'background-color:transparent;color:' + color +';'
  //   );
  // }
  // else{
  //   console.log(
  //     '%c> p5.js says: '+message+'%c [http://p5js.org/reference/#p5/'+func+
  //     ']', 'background-color:' + color + ';color:#FFF;',
  //     'background-color:transparent;color:' + color +';'
  //   );
  // }
}

var errorCases = {
  '0': {
    fileType: 'image',
    method: 'loadImage',
    message: ' hosting the image online,'
  },
  '1': {
    fileType: 'XML file',
    method: 'loadXML'
  },
  '2': {
    fileType: 'table file',
    method: 'loadTable'
  },
  '3': {
    fileType: 'text file',
    method: 'loadStrings'
  },
  '4': {
    fileType: 'font',
    method: 'loadFont',
    message: ' hosting the font online,'
  },
};
p5._friendlyFileLoadError = function (errorType, filePath) {
  var errorInfo = errorCases[ errorType ];
  var message = 'It looks like there was a problem' +
  ' loading your ' + errorInfo.fileType + '.' +
  ' Try checking if the file path%c [' + filePath + '] %cis correct,' +
  (errorInfo.message || '') + ' or running a local server.';
  report(message, errorInfo.method, FILE_LOAD);
};

function friendlyWelcome() {
  // p5.js brand - magenta: #ED225D
  var astrixBgColor = 'transparent';
  var astrixTxtColor = '#ED225D';
  var welcomeBgColor = '#ED225D';
  var welcomeTextColor = 'white';
  console.log(
  '%c    _ \n'+
  ' /\\| |/\\ \n'+
  ' \\ ` \' /  \n'+
  ' / , . \\  \n'+
  ' \\/|_|\\/ '+
  '\n\n%c> p5.js says: Welcome! '+
  'This is your friendly debugger. ' +
  'To turn me off switch to using “p5.min.js”.',
  'background-color:'+astrixBgColor+';color:' + astrixTxtColor +';',
  'background-color:'+welcomeBgColor+';color:' + welcomeTextColor +';'
  );
}

/**
 * Prints out all the colors in the color pallete with white text.
 * For color blindness testing.
 */
/* function testColors() {
  var str = 'A box of biscuits, a box of mixed biscuits and a biscuit mixer';
  report(str, 'print', '#ED225D'); // p5.js magenta
  report(str, 'print', '#2D7BB6'); // p5.js blue
  report(str, 'print', '#EE9900'); // p5.js orange
  report(str, 'print', '#A67F59'); // p5.js light brown
  report(str, 'print', '#704F21'); // p5.js gold
  report(str, 'print', '#1CC581'); // auto cyan
  report(str, 'print', '#FF6625'); // auto orange
  report(str, 'print', '#79EB22'); // auto green
  report(str, 'print', '#B40033'); // p5.js darkened magenta
  report(str, 'print', '#084B7F'); // p5.js darkened blue
  report(str, 'print', '#945F00'); // p5.js darkened orange
  report(str, 'print', '#6B441D'); // p5.js darkened brown
  report(str, 'print', '#2E1B00'); // p5.js darkened gold
  report(str, 'print', '#008851'); // auto dark cyan
  report(str, 'print', '#C83C00'); // auto dark orange
  report(str, 'print', '#4DB200'); // auto dark green
} */

// This is a lazily-defined list of p5 symbols that may be
// misused by beginners at top-level code, outside of setup/draw. We'd like
// to detect these errors and help the user by suggesting they move them
// into setup/draw.
//
// For more details, see https://github.com/processing/p5.js/issues/1121.
var misusedAtTopLevelCode = null;
var FAQ_URL = 'https://github.com/processing/p5.js/wiki/' +
              'Frequently-Asked-Questions' +
              '#why-cant-i-assign-variables-using-p5-functions-and-' +
              'variables-before-setup';

function defineMisusedAtTopLevelCode() {
  var uniqueNamesFound = {};

  var getSymbols = function(obj) {
    return Object.getOwnPropertyNames(obj).filter(function(name) {
      if (name[0] === '_') {
        return false;
      }
      if (name in uniqueNamesFound) {
        return false;
      }

      uniqueNamesFound[name] = true;

      return true;
    }).map(function(name) {
      var type;

      if (typeof(obj[name]) === 'function') {
        type = 'function';
      } else if (name === name.toUpperCase()) {
        type = 'constant';
      } else {
        type = 'variable';
      }

      return {name: name, type: type};
    });
  };

  misusedAtTopLevelCode = [].concat(
    getSymbols(p5.prototype),
    // At present, p5 only adds its constants to p5.prototype during
    // construction, which may not have happened at the time a
    // ReferenceError is thrown, so we'll manually add them to our list.
    getSymbols(_dereq_('./constants'))
  );

  // This will ultimately ensure that we report the most specific error
  // possible to the user, e.g. advising them about HALF_PI instead of PI
  // when their code misuses the former.
  misusedAtTopLevelCode.sort(function(a, b) {
    return b.name.length - a.name.length;
  });
}

function helpForMisusedAtTopLevelCode(e, log) {
  if (!log) {
    log = console.log.bind(console);
  }

  if (!misusedAtTopLevelCode) {
    defineMisusedAtTopLevelCode();
  }

  // If we find that we're logging lots of false positives, we can
  // uncomment the following code to avoid displaying anything if the
  // user's code isn't likely to be using p5's global mode. (Note that
  // setup/draw are more likely to be defined due to JS function hoisting.)
  //
  //if (!('setup' in window || 'draw' in window)) {
  //  return;
  //}

  misusedAtTopLevelCode.some(function(symbol) {
    // Note that while just checking for the occurrence of the
    // symbol name in the error message could result in false positives,
    // a more rigorous test is difficult because different browsers
    // log different messages, and the format of those messages may
    // change over time.
    //
    // For example, if the user uses 'PI' in their code, it may result
    // in any one of the following messages:
    //
    //   * 'PI' is undefined                           (Microsoft Edge)
    //   * ReferenceError: PI is undefined             (Firefox)
    //   * Uncaught ReferenceError: PI is not defined  (Chrome)

    if (e.message && e.message.match('\\W?'+symbol.name+'\\W') !== null) {
      log('%cDid you just try to use p5.js\'s ' + symbol.name +
          (symbol.type === 'function' ? '() ' : ' ') + symbol.type +
          '? If so, you may want to ' +
          'move it into your sketch\'s setup() function.\n\n' +
          'For more details, see: ' + FAQ_URL,
          'color: #B40033' /* Dark magenta */);
      return true;
    }
  });
}

// Exposing this primarily for unit testing.
p5.prototype._helpForMisusedAtTopLevelCode = helpForMisusedAtTopLevelCode;

if (document.readyState !== 'complete') {
  window.addEventListener('error', helpForMisusedAtTopLevelCode, false);

  // Our job is only to catch ReferenceErrors that are thrown when
  // global (non-instance mode) p5 APIs are used at the top-level
  // scope of a file, so we'll unbind our error listener now to make
  // sure we don't log false positives later.
  window.addEventListener('load', function() {
    window.removeEventListener('error', helpForMisusedAtTopLevelCode, false);
  });
}

module.exports = p5;

},{"./constants":53,"./core":54}],58:[function(_dereq_,module,exports){

var p5 = _dereq_('../core/core');


/**
 * _globalInit
 *
 * TODO: ???
 * if sketch is on window
 * assume "global" mode
 * and instantiate p5 automatically
 * otherwise do nothing
 *
 * @return {Undefined}
 */
var _globalInit = function() {
  if (!window.PHANTOMJS && !window.mocha) {
    // If there is a setup or draw function on the window
    // then instantiate p5 in "global" mode
    if(((window.setup && typeof window.setup === 'function') ||
       (window.draw && typeof window.draw === 'function')) &&
       !p5.instance) {
      new p5();
    }
  }
};

// TODO: ???
if (document.readyState === 'complete') {
  _globalInit();
} else {
  window.addEventListener('load', _globalInit , false);
}
},{"../core/core":54}],59:[function(_dereq_,module,exports){
/**
 * @module DOM
 * @submodule DOM
 * @for p5.Element
 */

var p5 = _dereq_('./core');

/**
 * Base class for all elements added to a sketch, including canvas,
 * graphics buffers, and other HTML elements. Methods in blue are
 * included in the core functionality, methods in brown are added
 * with the <a href="http://p5js.org/reference/#/libraries/p5.dom">p5.dom
 * library</a>.
 * It is not called directly, but p5.Element
 * objects are created by calling createCanvas, createGraphics,
 * or in the p5.dom library, createDiv, createImg, createInput, etc.
 *
 * @class p5.Element
 * @constructor
 * @param {String} elt DOM node that is wrapped
 * @param {p5} [pInst] pointer to p5 instance
 */
p5.Element = function(elt, pInst) {
  /**
   * Underlying HTML element. All normal HTML methods can be called on this.
   *
   * @property elt
   * @readOnly
   */
  this.elt = elt;
  this._pInst = pInst;
  this._events = {};
  this.width = this.elt.offsetWidth;
  this.height = this.elt.offsetHeight;
};

/**
 *
 * Attaches the element to the parent specified. A way of setting
 * the container for the element. Accepts either a string ID, DOM
 * node, or p5.Element. If no arguments given, parent node is returned.
 * For more ways to position the canvas, see the
 * <a href='https://github.com/processing/p5.js/wiki/Positioning-your-canvas'>
 * positioning the canvas</a> wiki page.
 *
 * @method parent
 * @param  {String|p5.Element|Object} parent the ID, DOM node, or p5.Element
 *                         of desired parent element
 * @chainable
 */
/**
 * @method parent
 * @return {p5.Element}
 *
 * @example
 * <div class="norender"><code>
 * // in the html file:
 * &lt;div id="myContainer">&lt;/div>
 * // in the js file:
 * var cnv = createCanvas(100, 100);
 * cnv.parent("myContainer");
 * </code></div>
 * <div class='norender'><code>
 * var div0 = createDiv('this is the parent');
 * var div1 = createDiv('this is the child');
 * div1.parent(div0); // use p5.Element
 * </code></div>
 * <div class='norender'><code>
 * var div0 = createDiv('this is the parent');
 * div0.id('apples');
 * var div1 = createDiv('this is the child');
 * div1.parent('apples'); // use id
 * </code></div>
 * <div class='norender'><code>
 * var elt = document.getElementById('myParentDiv');
 * var div1 = createDiv('this is the child');
 * div1.parent(elt); // use element from page
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.parent = function(p) {
  if (arguments.length === 0){
    return this.elt.parentNode;
  } else {
    if (typeof p === 'string') {
      if (p[0] === '#') {
        p = p.substring(1);
      }
      p = document.getElementById(p);
    } else if (p instanceof p5.Element) {
      p = p.elt;
    }
    p.appendChild(this.elt);
    return this;
  }
};

/**
 *
 * Sets the ID of the element. If no ID argument is passed in, it instead
 * returns the current ID of the element.
 *
 * @method id
 * @param  {String} id ID of the element
 * @chainable
 */
/**
 * @method id
 * @return {String} the id of the element
 *
 * @example
 * <div class='norender'><code>
 * function setup() {
 *   var cnv = createCanvas(100, 100);
 *   // Assigns a CSS selector ID to
 *   // the canvas element.
 *   cnv.id("mycanvas");
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.id = function(id) {
  if (arguments.length === 0) {
    return this.elt.id;
  } else {
    this.elt.id = id;
    this.width = this.elt.offsetWidth;
    this.height = this.elt.offsetHeight;
    return this;
  }
};

/**
 *
 * Adds given class to the element. If no class argument is passed in, it
 * instead returns a string containing the current class(es) of the element.
 *
 * @method class
 * @param  {String} class class to add
 * @chainable
 */
/**
 * @method class
 * @return {String} the class of the element
 */
p5.Element.prototype.class = function(c) {
  if (arguments.length === 0) {
    return this.elt.className;
  } else {
    this.elt.className = c;
    return this;
  }
};

/**
 * The .mousePressed() function is called once after every time a
 * mouse button is pressed over the element. This can be used to
 * attach element specific event listeners.
 *
 * @method mousePressed
 * @param  {function} fxn function to be fired when mouse is
 *                    pressed over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mousePressed(changeGray); // attach listener for
 *                                 // canvas click only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires with any click anywhere
 * function mousePressed() {
 *   d = d + 10;
 * }
 *
 * // this function fires only when cnv is clicked
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mousePressed = function (fxn) {
  attachListener('mousedown', fxn, this);
  attachListener('touchstart', fxn, this);
  return this;
};

/**
 * The .doubleClicked() function is called once after every time a
 * mouse button is pressed twice over the element. This can be used to
 * attach element and action specific event listeners.
 *
 * @method doubleClicked
 * @param  {Function} fxn function to be fired when mouse is
 *                    pressed over the element.
 * @return {p5.Element}
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.doubleClicked(changeGray); // attach listener for
 *                                 // canvas click only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires with any double click anywhere
 * function doubleClicked() {
 *   d = d + 10;
 * }
 *
 * // this function fires only when cnv is clicked
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.doubleClicked = function (fxn) {
  attachListener('doubleClicked', fxn, this);
  return this;
};


/**
 * The .mouseWheel() function is called once after every time a
 * mouse wheel is scrolled over the element. This can be used to
 * attach element specific event listeners.
 * <br><br>
 * The function accepts a callback function as argument which will be executed
 * when the `wheel` event is triggered on the element, the callback function is
 * passed one argument `event`. The `event.deltaY` property returns negative
 * values if the mouse wheel is rotated up or away from the user and positive
 * in the other direction. The `event.deltaX` does the same as `event.deltaY`
 * except it reads the horizontal wheel scroll of the mouse wheel.
 * <br><br>
 * On OS X with "natural" scrolling enabled, the `event.deltaY` values are
 * reversed.
 *
 * @method mouseWheel
 * @param  {function} fxn function to be fired when mouse wheel is
 *                    scrolled over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseWheel(changeSize); // attach listener for
 *                               // activity on canvas only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires with mousewheel movement
 * // anywhere on screen
 * function mouseWheel() {
 *   g = g + 10;
 * }
 *
 * // this function fires with mousewheel movement
 * // over canvas only
 * function changeSize(event) {
 *   if (event.deltaY > 0) {
 *     d = d + 10;
 *   } else {
 *     d = d - 10;
 *   }
 * }
 * </code></div>
 *
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseWheel = function (fxn) {
  attachListener('wheel', fxn, this);
  return this;
};

/**
 * The .mouseReleased() function is called once after every time a
 * mouse button is released over the element. This can be used to
 * attach element specific event listeners.
 *
 * @method mouseReleased
 * @param  {function} fxn function to be fired when mouse is
 *                    released over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseReleased(changeGray); // attach listener for
 *                                  // activity on canvas only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires after the mouse has been
 * // released
 * function mouseReleased() {
 *   d = d + 10;
 * }
 *
 * // this function fires after the mouse has been
 * // released while on canvas
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseReleased = function (fxn) {
  attachListener('mouseup', fxn, this);
  attachListener('touchend', fxn, this);
  return this;
};


/**
 * The .mouseClicked() function is called once after a mouse button is
 * pressed and released over the element. This can be used to
 * attach element specific event listeners.
 *
 * @method mouseClicked
 * @param  {function} fxn function to be fired when mouse is
 *                    clicked over the element.
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var cnv;
 * var d;
 * var g;
 *
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseClicked(changeGray); // attach listener for
 *                                 // activity on canvas only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires after the mouse has been
 * // clicked anywhere
 * function mouseClicked() {
 *   d = d + 10;
 * }
 *
 * // this function fires after the mouse has been
 * // clicked on canvas
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code>
 * </div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseClicked = function (fxn) {
  attachListener('click', fxn, this);
  return this;
};

/**
 * The .mouseMoved() function is called once every time a
 * mouse moves over the element. This can be used to attach an
 * element specific event listener.
 *
 * @method mouseMoved
 * @param  {function} fxn function to be fired when mouse is
 *                    moved over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d = 30;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseMoved(changeSize); // attach listener for
 *                               // activity on canvas only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   fill(200);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires when mouse moves anywhere on
 * // page
 * function mouseMoved() {
 *   g = g + 5;
 *   if (g > 255) {
 *     g = 0;
 *   }
 * }
 *
 * // this function fires when mouse moves over canvas
 * function changeSize() {
 *   d = d + 2;
 *   if (d > 100) {
 *     d = 0;
 *   }
 * }
 * </code></div>
 *
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseMoved = function (fxn) {
  attachListener('mousemove', fxn, this);
  attachListener('touchmove', fxn, this);
  return this;
};

/**
 * The .mouseOver() function is called once after every time a
 * mouse moves onto the element. This can be used to attach an
 * element specific event listener.
 *
 * @method mouseOver
 * @param  {function} fxn function to be fired when mouse is
 *                    moved over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseOver(changeGray);
 *   d = 10;
 * }
 *
 * function draw() {
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * function changeGray() {
 *   d = d + 10;
 *   if (d > 100) {
 *     d = 0;
 *   }
 * }
 * </code></div>
 *
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseOver = function (fxn) {
  attachListener('mouseover', fxn, this);
  return this;
};


/**
 * The .changed() function is called when the value of an
 * element is changed.
 * This can be used to attach an element specific event listener.
 *
 * @method changed
 * @param  {function} fxn function to be fired when the value of an
 * element changes.
 * @chainable
 * @example
 * <div><code>
 * var sel;
 *
 * function setup() {
 *   textAlign(CENTER);
 *   background(200);
 *   sel = createSelect();
 *   sel.position(10, 10);
 *   sel.option('pear');
 *   sel.option('kiwi');
 *   sel.option('grape');
 *   sel.changed(mySelectEvent);
 * }
 *
 * function mySelectEvent() {
 *   var item = sel.value();
 *   background(200);
 *   text("it's a "+item+"!", 50, 50);
 * }
 * </code></div>
 * <div><code>
 * var checkbox;
 * var cnv;
 *
 * function setup() {
 *   checkbox = createCheckbox(" fill");
 *   checkbox.changed(changeFill);
 *   cnv = createCanvas(100, 100);
 *   cnv.position(0, 30);
 *   noFill();
 * }
 *
 * function draw() {
 *   background(200);
 *   ellipse(50, 50, 50, 50);
 * }
 *
 * function changeFill() {
 *   if (checkbox.checked()) {
 *     fill(0);
 *   } else {
 *     noFill();
 *   }
 * }
 * </code></div>
 *
 * @alt
 * dropdown: pear, kiwi, grape. When selected text "its a" + selection shown.
 *
 */
p5.Element.prototype.changed = function (fxn) {
  attachListener('change', fxn, this);
  return this;
};

/**
 * The .input() function is called when any user input is
 * detected with an element. The input event is often used
 * to detect keystrokes in a input element, or changes on a
 * slider element. This can be used to attach an element specific
 * event listener.
 *
 * @method input
 * @param  {function} fxn function to be fired on user input.
 * @chainable
 * @example
 * <div class='norender'><code>
 * // Open your console to see the output
 * function setup() {
 *   var inp = createInput('');
 *   inp.input(myInputEvent);
 * }
 *
 * function myInputEvent() {
 *   console.log('you are typing: ', this.value());
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.input = function (fxn) {
  attachListener('input', fxn, this);
  return this;
};

/**
 * The .mouseOut() function is called once after every time a
 * mouse moves off the element. This can be used to attach an
 * element specific event listener.
 *
 * @method mouseOut
 * @param  {function} fxn function to be fired when mouse is
 *                    moved off the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.mouseOut(changeGray);
 *   d = 10;
 * }
 *
 * function draw() {
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * function changeGray() {
 *   d = d + 10;
 *   if (d > 100) {
 *     d = 0;
 *   }
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.mouseOut = function (fxn) {
  attachListener('mouseout', fxn, this);
  return this;
};

/**
 * The .touchStarted() function is called once after every time a touch is
 * registered. This can be used to attach element specific event listeners.
 *
 * @method touchStarted
 * @param  {function} fxn function to be fired when touch is
 *                    started over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.touchStarted(changeGray); // attach listener for
 *                                 // canvas click only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires with any touch anywhere
 * function touchStarted() {
 *   d = d + 10;
 * }
 *
 * // this function fires only when cnv is clicked
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.touchStarted = function (fxn) {
  attachListener('touchstart', fxn, this);
  attachListener('mousedown', fxn, this);
  return this;
};

/**
 * The .touchMoved() function is called once after every time a touch move is
 * registered. This can be used to attach element specific event listeners.
 *
 * @method touchMoved
 * @param  {function} fxn function to be fired when touch is moved
 *                    over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.touchMoved(changeGray); // attach listener for
 *                               // canvas click only
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 * }
 *
 * // this function fires only when cnv is clicked
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.touchMoved = function (fxn) {
  attachListener('touchmove', fxn, this);
  attachListener('mousemove', fxn, this);
  return this;
};

/**
 * The .touchEnded() function is called once after every time a touch is
 * registered. This can be used to attach element specific event listeners.
 *
 * @method touchEnded
 * @param  {function} fxn function to be fired when touch is
 *                    ended over the element.
 * @chainable
 * @example
 * <div class='norender'><code>
 * var cnv;
 * var d;
 * var g;
 * function setup() {
 *   cnv = createCanvas(100, 100);
 *   cnv.touchEnded(changeGray);   // attach listener for
 *                                 // canvas click only
 *   d = 10;
 *   g = 100;
 * }
 *
 * function draw() {
 *   background(g);
 *   ellipse(width/2, height/2, d, d);
 * }
 *
 * // this function fires with any touch anywhere
 * function touchEnded() {
 *   d = d + 10;
 * }
 *
 * // this function fires only when cnv is clicked
 * function changeGray() {
 *   g = random(0, 255);
 * }
 * </code></div>
 *
 *
 * @alt
 * no display.
 *
 */
p5.Element.prototype.touchEnded = function (fxn) {
  attachListener('touchend', fxn, this);
  attachListener('mouseup', fxn, this);
  return this;
};



/**
 * The .dragOver() function is called once after every time a
 * file is dragged over the element. This can be used to attach an
 * element specific event listener.
 *
 * @method dragOver
 * @param  {function} fxn function to be fired when mouse is
 *                    dragged over the element.
 * @chainable
 * @example
 * <div><code>
 * // To test this sketch, simply drag a
 * // file over the canvas
 * function setup() {
 *   var c = createCanvas(100, 100);
 *   background(200);
 *   textAlign(CENTER);
 *   text('Drag file', width/2, height/2);
 *   c.dragOver(dragOverCallback);
 * }
 *
 * // This function will be called whenever
 * // a file is dragged over the canvas
 * function dragOverCallback() {
 *   background(240);
 *   text('Dragged over', width/2, height/2);
 * }
 * </code></div>
 * @alt
 * nothing displayed
 */
p5.Element.prototype.dragOver = function (fxn) {
  attachListener('dragover', fxn, this);
  return this;
};

/**
 * The .dragLeave() function is called once after every time a
 * dragged file leaves the element area. This can be used to attach an
 * element specific event listener.
 *
 * @method dragLeave
 * @param  {function} fxn function to be fired when mouse is
 *                    dragged over the element.
 * @chainable
 * @example
 * <div><code>
 * // To test this sketch, simply drag a file
 * // over and then out of the canvas area
 * function setup() {
 *   var c = createCanvas(100, 100);
 *   background(200);
 *   textAlign(CENTER);
 *   text('Drag file', width/2, height/2);
 *   c.dragLeave(dragLeaveCallback);
 * }
 *
 * // This function will be called whenever
 * // a file is dragged out of the canvas
 * function dragLeaveCallback() {
 *   background(240);
 *   text('Dragged off', width/2, height/2);
 * }
 * </code></div>
 * @alt
 * nothing displayed
 */
p5.Element.prototype.dragLeave = function (fxn) {
  attachListener('dragleave', fxn, this);
  return this;
};

/**
 * The .drop() function is called for each file dropped on the element.
 * It requires a callback that is passed a p5.File object.  You can
 * optionally pass two callbacks, the first one (required) is triggered
 * for each file dropped when the file is loaded.  The second (optional)
 * is triggered just once when a file (or files) are dropped.
 *
 * @method drop
 * @param  {function} callback  callback triggered when files are dropped.
 * @param  {function} fxn       callback to receive loaded file.
 * @chainable
 * @example
 * <div><code>
 * function setup() {
 *   var c = createCanvas(100, 100);
 *   background(200);
 *   textAlign(CENTER);
 *   text('drop image', width/2, height/2);
 *   c.drop(gotFile);
 * }
 *
 * function gotFile(file) {
 *   var img = createImg(file.data).hide();
 *   // Draw the image onto the canvas
 *   image(img, 0, 0, width, height);
 * }
 * </code></div>
 *
 * @alt
 * Canvas turns into whatever image is dragged/dropped onto it.
 *
 */
p5.Element.prototype.drop = function (callback, fxn) {
  // Make a file loader callback and trigger user's callback
  function makeLoader(theFile) {
    // Making a p5.File object
    var p5file = new p5.File(theFile);
    return function(e) {
      p5file.data = e.target.result;
      callback(p5file);
    };
  }

  // Is the file stuff supported?
  if (window.File && window.FileReader && window.FileList && window.Blob) {

    // If you want to be able to drop you've got to turn off
    // a lot of default behavior
    attachListener('dragover',function(evt) {
      evt.stopPropagation();
      evt.preventDefault();
    },this);

    // If this is a drag area we need to turn off the default behavior
    attachListener('dragleave',function(evt) {
      evt.stopPropagation();
      evt.preventDefault();
    },this);

    // If just one argument it's the callback for the files
    if (arguments.length > 1) {
      attachListener('drop', fxn, this);
    }

    // Deal with the files
    attachListener('drop', function(evt) {

      evt.stopPropagation();
      evt.preventDefault();

      // A FileList
      var files = evt.dataTransfer.files;

      // Load each one and trigger the callback
      for (var i = 0; i < files.length; i++) {
        var f = files[i];
        var reader = new FileReader();
        reader.onload = makeLoader(f);


        // Text or data?
        // This should likely be improved
        if (f.type.indexOf('text') > -1) {
          reader.readAsText(f);
        } else {
          reader.readAsDataURL(f);
        }
      }
    }, this);
  } else {
    console.log('The File APIs are not fully supported in this browser.');
  }

  return this;
};




function attachListener(ev, fxn, ctx) {
  // LM removing, not sure why we had this?
  // var _this = ctx;
  // var f = function (e) { fxn(e, _this); };
  var f = fxn.bind(ctx);
  ctx.elt.addEventListener(ev, f, false);
  ctx._events[ev] = f;
}

/**
 * Helper fxn for sharing pixel methods
 *
 */
p5.Element.prototype._setProperty = function (prop, value) {
  this[prop] = value;
};


module.exports = p5.Element;

},{"./core":54}],60:[function(_dereq_,module,exports){
/**
 * @module Rendering
 * @submodule Rendering
 * @for p5
 */

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');

/**
 * Thin wrapper around a renderer, to be used for creating a
 * graphics buffer object. Use this class if you need
 * to draw into an off-screen graphics buffer. The two parameters define the
 * width and height in pixels. The fields and methods for this class are
 * extensive, but mirror the normal drawing API for p5.
 *
 * @class p5.Graphics
 * @constructor
 * @extends p5.Element
 * @param {Number} w            width
 * @param {Number} h            height
 * @param {Constant} renderer   the renderer to use, either P2D or WEBGL
 * @param {p5} [pInst]          pointer to p5 instance
 */
p5.Graphics = function(w, h, renderer, pInst) {

  var r = renderer || constants.P2D;

  this.canvas = document.createElement('canvas');
  var node = this._userNode || document.body;
  node.appendChild(this.canvas);

  p5.Element.call(this, this.canvas, pInst, false);
  this._styles = [];
  this.width = w;
  this.height = h;
  this._pixelDensity = pInst._pixelDensity;

  if (r === constants.WEBGL) {
    this._renderer = new p5.RendererGL(this.canvas, this, false);
  } else {
    this._renderer = new p5.Renderer2D(this.canvas, this, false);
  }

  this._renderer.resize(w, h);
  this._renderer._applyDefaults();

  pInst._elements.push(this);

  // bind methods and props of p5 to the new object
  for (var p in p5.prototype) {
    if (!this[p]) {
      if (typeof p5.prototype[p] === 'function') {
        this[p] = p5.prototype[p].bind(this);
      } else {
        this[p] = p5.prototype[p];
      }
    }
  }

  return this;
};

p5.Graphics.prototype = Object.create(p5.Element.prototype);

p5.Graphics.prototype.remove = function() {
  if (this.elt.parentNode) {
    this.elt.parentNode.removeChild(this.elt);
  }
  for (var elt_ev in this._events) {
    this.elt.removeEventListener(elt_ev, this._events[elt_ev]);
  }
};

module.exports = p5.Graphics;

},{"./constants":53,"./core":54}],61:[function(_dereq_,module,exports){
/**
 * @module Rendering
 * @submodule Rendering
 * @for p5
 */

var p5 = _dereq_('./core');
var constants = _dereq_('../core/constants');

/**
 * Main graphics and rendering context, as well as the base API
 * implementation for p5.js "core". To be used as the superclass for
 * Renderer2D and Renderer3D classes, respecitvely.
 *
 * @class p5.Renderer
 * @constructor
 * @extends p5.Element
 * @param {String} elt DOM node that is wrapped
 * @param {p5} [pInst] pointer to p5 instance
 * @param {Boolean} [isMainCanvas] whether we're using it as main canvas
 */
p5.Renderer = function(elt, pInst, isMainCanvas) {
  p5.Element.call(this, elt, pInst);
  this.canvas = elt;
  this._pInst = pInst;
  if (isMainCanvas) {
    this._isMainCanvas = true;
    // for pixel method sharing with pimage
    this._pInst._setProperty('_curElement', this);
    this._pInst._setProperty('canvas', this.canvas);
    this._pInst._setProperty('width', this.width);
    this._pInst._setProperty('height', this.height);
  } else { // hide if offscreen buffer by default
    this.canvas.style.display = 'none';
    this._styles = []; // non-main elt styles stored in p5.Renderer
  }


  this._textSize = 12;
  this._textLeading = 15;
  this._textFont = 'sans-serif';
  this._textStyle = constants.NORMAL;
  this._textAscent = null;
  this._textDescent = null;


  this._rectMode = constants.CORNER;
  this._ellipseMode = constants.CENTER;
  this._curveTightness = 0;
  this._imageMode = constants.CORNER;

  this._tint = null;
  this._doStroke = true;
  this._doFill = true;
  this._strokeSet = false;
  this._fillSet = false;
  this._colorMode = constants.RGB;
  this._colorMaxes = {
    rgb: [255, 255, 255, 255],
    hsb: [360, 100, 100, 1],
    hsl: [360, 100, 100, 1]
  };

};

p5.Renderer.prototype = Object.create(p5.Element.prototype);




/**
 * Resize our canvas element.
 */
p5.Renderer.prototype.resize = function(w, h) {
  this.width = w;
  this.height = h;
  this.elt.width = w * this._pInst._pixelDensity;
  this.elt.height = h * this._pInst._pixelDensity;
  this.elt.style.width = w +'px';
  this.elt.style.height = h + 'px';
  if (this._isMainCanvas) {
    this._pInst._setProperty('width', this.width);
    this._pInst._setProperty('height', this.height);
  }
};

p5.Renderer.prototype.textLeading = function(l) {

  if (arguments.length && arguments[0]) {

    this._setProperty('_textLeading', l);
    return this;
  }

  return this._textLeading;
};

p5.Renderer.prototype.textSize = function(s) {

  if (arguments.length && arguments[0]) {

    this._setProperty('_textSize', s);
    this._setProperty('_textLeading', s * constants._DEFAULT_LEADMULT);
    return this._applyTextProperties();
  }

  return this._textSize;
};

p5.Renderer.prototype.textStyle = function(s) {

  if (arguments.length && arguments[0]) {

    if (s === constants.NORMAL ||
      s === constants.ITALIC ||
      s === constants.BOLD) {
      this._setProperty('_textStyle', s);
    }

    return this._applyTextProperties();
  }

  return this._textStyle;
};

p5.Renderer.prototype.textAscent = function() {
  if (this._textAscent === null) {
    this._updateTextMetrics();
  }
  return this._textAscent;
};

p5.Renderer.prototype.textDescent = function() {

  if (this._textDescent === null) {
    this._updateTextMetrics();
  }
  return this._textDescent;
};

p5.Renderer.prototype._applyDefaults = function(){
  return this;
};

/**
 * Helper fxn to check font type (system or otf)
 */
p5.Renderer.prototype._isOpenType = function(f) {

  f = f || this._textFont;
  return (typeof f === 'object' && f.font && f.font.supported);
};

p5.Renderer.prototype._updateTextMetrics = function() {

  if (this._isOpenType()) {

    this._setProperty('_textAscent', this._textFont._textAscent());
    this._setProperty('_textDescent', this._textFont._textDescent());
    return this;
  }

  // Adapted from http://stackoverflow.com/a/25355178
  var text = document.createElement('span');
  text.style.fontFamily = this._textFont;
  text.style.fontSize = this._textSize + 'px';
  text.innerHTML = 'ABCjgq|';

  var block = document.createElement('div');
  block.style.display = 'inline-block';
  block.style.width = '1px';
  block.style.height = '0px';

  var container = document.createElement('div');
  container.appendChild(text);
  container.appendChild(block);

  container.style.height = '0px';
  container.style.overflow = 'hidden';
  document.body.appendChild(container);

  block.style.verticalAlign = 'baseline';
  var blockOffset = calculateOffset(block);
  var textOffset = calculateOffset(text);
  var ascent = blockOffset[1] - textOffset[1];

  block.style.verticalAlign = 'bottom';
  blockOffset = calculateOffset(block);
  textOffset = calculateOffset(text);
  var height = blockOffset[1] - textOffset[1];
  var descent = height - ascent;

  document.body.removeChild(container);

  this._setProperty('_textAscent', ascent);
  this._setProperty('_textDescent', descent);

  return this;
};

/**
 * Helper fxn to measure ascent and descent.
 * Adapted from http://stackoverflow.com/a/25355178
 */
function calculateOffset(object) {
  var currentLeft = 0,
    currentTop = 0;
  if (object.offsetParent) {
    do {
      currentLeft += object.offsetLeft;
      currentTop += object.offsetTop;
    } while (object = object.offsetParent);
  } else {
    currentLeft += object.offsetLeft;
    currentTop += object.offsetTop;
  }
  return [currentLeft, currentTop];
}

module.exports = p5.Renderer;

},{"../core/constants":53,"./core":54}],62:[function(_dereq_,module,exports){

var p5 = _dereq_('./core');
var canvas = _dereq_('./canvas');
var constants = _dereq_('./constants');
var filters = _dereq_('../image/filters');

_dereq_('./p5.Renderer');

/**
 * p5.Renderer2D
 * The 2D graphics canvas renderer class.
 * extends p5.Renderer
 */
var styleEmpty = 'rgba(0,0,0,0)';
// var alphaThreshold = 0.00125; // minimum visible

p5.Renderer2D = function(elt, pInst, isMainCanvas){
  p5.Renderer.call(this, elt, pInst, isMainCanvas);
  this.drawingContext = this.canvas.getContext('2d');
  this._pInst._setProperty('drawingContext', this.drawingContext);
  return this;
};

p5.Renderer2D.prototype = Object.create(p5.Renderer.prototype);

p5.Renderer2D.prototype._applyDefaults = function() {
  this._setFill(constants._DEFAULT_FILL);
  this._setStroke(constants._DEFAULT_STROKE);
  this.drawingContext.lineCap = constants.ROUND;
  this.drawingContext.font = 'normal 12px sans-serif';
};

p5.Renderer2D.prototype.resize = function(w,h) {
  p5.Renderer.prototype.resize.call(this, w,h);
  this.drawingContext.scale(this._pInst._pixelDensity,
                            this._pInst._pixelDensity);
};

//////////////////////////////////////////////
// COLOR | Setting
//////////////////////////////////////////////

p5.Renderer2D.prototype.background = function() {
  this.drawingContext.save();
  this.drawingContext.setTransform(1, 0, 0, 1, 0, 0);
  this.drawingContext.scale(this._pInst._pixelDensity,
                            this._pInst._pixelDensity);

  if (arguments[0] instanceof p5.Image) {
    this._pInst.image(arguments[0], 0, 0, this.width, this.height);
  } else {
    var curFill = this._getFill();
    // create background rect
    var color = this._pInst.color.apply(this, arguments);
    var newFill = color.toString();
    this._setFill(newFill);
    this.drawingContext.fillRect(0, 0, this.width, this.height);
    // reset fill
    this._setFill(curFill);
  }
  this.drawingContext.restore();
};

p5.Renderer2D.prototype.clear = function() {
  this.drawingContext.clearRect(0, 0, this.width, this.height);
};

p5.Renderer2D.prototype.fill = function() {
  var color = this._pInst.color.apply(this, arguments);
  this._setFill(color.toString());
};

p5.Renderer2D.prototype.stroke = function() {
  var color = this._pInst.color.apply(this, arguments);
  this._setStroke(color.toString());
};

//////////////////////////////////////////////
// IMAGE | Loading & Displaying
//////////////////////////////////////////////

p5.Renderer2D.prototype.image =
  function (img, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight) {
  var cnv;
  try {
    if (this._tint) {
      if (p5.MediaElement && img instanceof p5.MediaElement) {
        img.loadPixels();
      }
      if (img.canvas) {
        cnv = this._getTintedImageCanvas(img);
      }
    }
    if (!cnv) {
      cnv = img.canvas || img.elt;
    }
    this.drawingContext.drawImage(cnv, sx, sy, sWidth, sHeight, dx, dy,
      dWidth, dHeight);
  } catch (e) {
    if (e.name !== 'NS_ERROR_NOT_AVAILABLE') {
      throw e;
    }
  }
};

p5.Renderer2D.prototype._getTintedImageCanvas = function (img) {
  if (!img.canvas) {
    return img;
  }
  var pixels = filters._toPixels(img.canvas);
  var tmpCanvas = document.createElement('canvas');
  tmpCanvas.width = img.canvas.width;
  tmpCanvas.height = img.canvas.height;
  var tmpCtx = tmpCanvas.getContext('2d');
  var id = tmpCtx.createImageData(img.canvas.width, img.canvas.height);
  var newPixels = id.data;
  for (var i = 0; i < pixels.length; i += 4) {
    var r = pixels[i];
    var g = pixels[i + 1];
    var b = pixels[i + 2];
    var a = pixels[i + 3];
    newPixels[i] = r * this._tint[0] / 255;
    newPixels[i + 1] = g * this._tint[1] / 255;
    newPixels[i + 2] = b * this._tint[2] / 255;
    newPixels[i + 3] = a * this._tint[3] / 255;
  }
  tmpCtx.putImageData(id, 0, 0);
  return tmpCanvas;
};


//////////////////////////////////////////////
// IMAGE | Pixels
//////////////////////////////////////////////

p5.Renderer2D.prototype.blendMode = function(mode) {
  this.drawingContext.globalCompositeOperation = mode;
};
p5.Renderer2D.prototype.blend = function() {
  var currBlend = this.drawingContext.globalCompositeOperation;
  var blendMode = arguments[arguments.length - 1];

  var copyArgs = Array.prototype.slice.call(
    arguments,
    0,
    arguments.length - 1
  );

  this.drawingContext.globalCompositeOperation = blendMode;
  if (this._pInst) {
    this._pInst.copy.apply(this._pInst, copyArgs);
  } else {
    this.copy.apply(this, copyArgs);
  }
  this.drawingContext.globalCompositeOperation = currBlend;
};

p5.Renderer2D.prototype.copy = function () {
  var srcImage, sx, sy, sw, sh, dx, dy, dw, dh;
  if (arguments.length === 9) {
    srcImage = arguments[0];
    sx = arguments[1];
    sy = arguments[2];
    sw = arguments[3];
    sh = arguments[4];
    dx = arguments[5];
    dy = arguments[6];
    dw = arguments[7];
    dh = arguments[8];
  } else if (arguments.length === 8) {
    srcImage = this._pInst;
    sx = arguments[0];
    sy = arguments[1];
    sw = arguments[2];
    sh = arguments[3];
    dx = arguments[4];
    dy = arguments[5];
    dw = arguments[6];
    dh = arguments[7];
  } else {
    throw new Error('Signature not supported');
  }
  p5.Renderer2D._copyHelper(srcImage, sx, sy, sw, sh, dx, dy, dw, dh);
};

p5.Renderer2D._copyHelper =
function (srcImage, sx, sy, sw, sh, dx, dy, dw, dh) {
  srcImage.loadPixels();
  var s = srcImage.canvas.width / srcImage.width;
  this.drawingContext.drawImage(srcImage.canvas,
    s * sx, s * sy, s * sw, s * sh, dx, dy, dw, dh);
};

p5.Renderer2D.prototype.get = function(x, y, w, h) {
  if (x === undefined && y === undefined &&
      w === undefined && h === undefined){
    x = 0;
    y = 0;
    w = this.width;
    h = this.height;
  } else if (w === undefined && h === undefined) {
    w = 1;
    h = 1;
  }

  // if the section does not overlap the canvas
  if(x + w < 0 || y + h < 0 || x > this.width || y > this.height){
    return [0, 0, 0, 255];
  }

  var ctx = this._pInst || this;
  ctx.loadPixels();

  var pd = ctx._pixelDensity;

  // round down to get integer numbers
  x = Math.floor(x);
  y = Math.floor(y);
  w = Math.floor(w);
  h = Math.floor(h);

  var sx = x * pd;
  var sy = y * pd;
  if (w === 1 && h === 1){
    var imageData = this.drawingContext.getImageData(sx, sy, 1, 1).data;
    //imageData = [0,0,0,0];
    return [
      imageData[0],
      imageData[1],
      imageData[2],
      imageData[3]
    ];
  } else {
    //auto constrain the width and height to
    //dimensions of the source image
    var dw = Math.min(w, ctx.width);
    var dh = Math.min(h, ctx.height);
    var sw = dw * pd;
    var sh = dh * pd;

    var region = new p5.Image(dw, dh);
    region.canvas.getContext('2d').drawImage(this.canvas, sx, sy, sw, sh,
      0, 0, dw, dh);

    return region;
  }
};

p5.Renderer2D.prototype.loadPixels = function () {
  var pd = this._pixelDensity || this._pInst._pixelDensity;
  var w = this.width * pd;
  var h = this.height * pd;
  var imageData = this.drawingContext.getImageData(0, 0, w, h);
  // @todo this should actually set pixels per object, so diff buffers can
  // have diff pixel arrays.
  if (this._pInst) {
    this._pInst._setProperty('imageData', imageData);
    this._pInst._setProperty('pixels', imageData.data);
  } else { // if called by p5.Image
    this._setProperty('imageData', imageData);
    this._setProperty('pixels', imageData.data);
  }
};

p5.Renderer2D.prototype.set = function (x, y, imgOrCol) {
  // round down to get integer numbers
  x = Math.floor(x);
  y = Math.floor(y);
  if (imgOrCol instanceof p5.Image) {
    this.drawingContext.save();
    this.drawingContext.setTransform(1, 0, 0, 1, 0, 0);
    this.drawingContext.scale(this._pInst._pixelDensity,
      this._pInst._pixelDensity);
    this.drawingContext.drawImage(imgOrCol.canvas, x, y);
    this.loadPixels.call(this._pInst);
    this.drawingContext.restore();
  } else {
    var ctx = this._pInst || this;
    var r = 0, g = 0, b = 0, a = 0;
    var idx = 4*((y * ctx._pixelDensity) *
      (this.width * ctx._pixelDensity) + (x * ctx._pixelDensity));
    if (!ctx.imageData) {
      ctx.loadPixels.call(ctx);
    }
    if (typeof imgOrCol === 'number') {
      if (idx < ctx.pixels.length) {
        r = imgOrCol;
        g = imgOrCol;
        b = imgOrCol;
        a = 255;
        //this.updatePixels.call(this);
      }
    }
    else if (imgOrCol instanceof Array) {
      if (imgOrCol.length < 4) {
        throw new Error('pixel array must be of the form [R, G, B, A]');
      }
      if (idx < ctx.pixels.length) {
        r = imgOrCol[0];
        g = imgOrCol[1];
        b = imgOrCol[2];
        a = imgOrCol[3];
        //this.updatePixels.call(this);
      }
    } else if (imgOrCol instanceof p5.Color) {
      if (idx < ctx.pixels.length) {
        r = imgOrCol.levels[0];
        g = imgOrCol.levels[1];
        b = imgOrCol.levels[2];
        a = imgOrCol.levels[3];
        //this.updatePixels.call(this);
      }
    }
    // loop over pixelDensity * pixelDensity
    for (var i = 0; i < ctx._pixelDensity; i++) {
      for (var j = 0; j < ctx._pixelDensity; j++) {
        // loop over
        idx = 4*((y * ctx._pixelDensity + j) * this.width *
          ctx._pixelDensity + (x * ctx._pixelDensity + i));
        ctx.pixels[idx] = r;
        ctx.pixels[idx+1] = g;
        ctx.pixels[idx+2] = b;
        ctx.pixels[idx+3] = a;
      }
    }
  }
};

p5.Renderer2D.prototype.updatePixels = function (x, y, w, h) {
  var pd = this._pixelDensity || this._pInst._pixelDensity;
  if (x === undefined &&
      y === undefined &&
      w === undefined &&
      h === undefined) {
    x = 0;
    y = 0;
    w = this.width;
    h = this.height;
  }
  w *= pd;
  h *= pd;

  if (this._pInst) {
    this.drawingContext.putImageData(this._pInst.imageData, x, y, 0, 0, w, h);
  } else {
    this.drawingContext.putImageData(this.imageData, x, y, 0, 0, w, h);
  }
};

//////////////////////////////////////////////
// SHAPE | 2D Primitives
//////////////////////////////////////////////

/**
 * Generate a cubic Bezier representing an arc on the unit circle of total
 * angle `size` radians, beginning `start` radians above the x-axis. Up to
 * four of these curves are combined to make a full arc.
 *
 * See www.joecridge.me/bezier.pdf for an explanation of the method.
 */
p5.Renderer2D.prototype._acuteArcToBezier =
  function _acuteArcToBezier(start, size) {
  // Evauate constants.
  var alpha = size / 2.0,
    cos_alpha = Math.cos(alpha),
    sin_alpha = Math.sin(alpha),
    cot_alpha = 1.0 / Math.tan(alpha),
    phi = start + alpha,  // This is how far the arc needs to be rotated.
    cos_phi = Math.cos(phi),
    sin_phi = Math.sin(phi),
    lambda = (4.0 - cos_alpha) / 3.0,
    mu = sin_alpha + (cos_alpha - lambda) * cot_alpha;

  // Return rotated waypoints.
  return {
    ax: Math.cos(start),
    ay: Math.sin(start),
    bx: lambda * cos_phi + mu * sin_phi,
    by: lambda * sin_phi - mu * cos_phi,
    cx: lambda * cos_phi - mu * sin_phi,
    cy: lambda * sin_phi + mu * cos_phi,
    dx: Math.cos(start + size),
    dy: Math.sin(start + size)
  };
};

p5.Renderer2D.prototype.arc =
  function(x, y, w, h, start, stop, mode) {
  var ctx = this.drawingContext;
  var vals = canvas.arcModeAdjust(x, y, w, h, this._ellipseMode);
  var rx = vals.w / 2.0;
  var ry = vals.h / 2.0;
  var epsilon = 0.00001;  // Smallest visible angle on displays up to 4K.
  var arcToDraw = 0;
  var curves = [];

  // Create curves
  while(stop - start > epsilon) {
    arcToDraw = Math.min(stop - start, constants.HALF_PI);
    curves.push(this._acuteArcToBezier(start, arcToDraw));
    start += arcToDraw;
  }

  // Fill curves
  if (this._doFill) {
    ctx.beginPath();
    curves.forEach(function (curve, index) {
      if (index === 0) {
        ctx.moveTo(vals.x + curve.ax * rx, vals.y + curve.ay * ry);
      }
      ctx.bezierCurveTo(vals.x + curve.bx * rx, vals.y + curve.by * ry,
                        vals.x + curve.cx * rx, vals.y + curve.cy * ry,
                        vals.x + curve.dx * rx, vals.y + curve.dy * ry);
    });
    if (mode === constants.PIE || mode == null) {
      ctx.lineTo(vals.x, vals.y);
    }
    ctx.closePath();
    ctx.fill();
  }

  // Stroke curves
  if (this._doStroke) {
    ctx.beginPath();
    curves.forEach(function (curve, index) {
      if (index === 0) {
        ctx.moveTo(vals.x + curve.ax * rx, vals.y + curve.ay * ry);
      }
      ctx.bezierCurveTo(vals.x + curve.bx * rx, vals.y + curve.by * ry,
                        vals.x + curve.cx * rx, vals.y + curve.cy * ry,
                        vals.x + curve.dx * rx, vals.y + curve.dy * ry);
    });
    if (mode === constants.PIE) {
      ctx.lineTo(vals.x, vals.y);
      ctx.closePath();
    } else if (mode === constants.CHORD) {
      ctx.closePath();
    }
    ctx.stroke();
  }
  return this;
};

p5.Renderer2D.prototype.ellipse = function(args) {
  var ctx = this.drawingContext;
  var doFill = this._doFill, doStroke = this._doStroke;
  var x = args[0],
    y = args[1],
    w = args[2],
    h = args[3];
  if (doFill && !doStroke) {
    if(this._getFill() === styleEmpty) {
      return this;
    }
  } else if (!doFill && doStroke) {
    if(this._getStroke() === styleEmpty) {
      return this;
    }
  }
  var kappa = 0.5522847498,
    ox = (w / 2) * kappa, // control point offset horizontal
    oy = (h / 2) * kappa, // control point offset vertical
    xe = x + w,      // x-end
    ye = y + h,      // y-end
    xm = x + w / 2,  // x-middle
    ym = y + h / 2;  // y-middle
  ctx.beginPath();
  ctx.moveTo(x, ym);
  ctx.bezierCurveTo(x, ym - oy, xm - ox, y, xm, y);
  ctx.bezierCurveTo(xm + ox, y, xe, ym - oy, xe, ym);
  ctx.bezierCurveTo(xe, ym + oy, xm + ox, ye, xm, ye);
  ctx.bezierCurveTo(xm - ox, ye, x, ym + oy, x, ym);
  ctx.closePath();
  if (doFill) {
    ctx.fill();
  }
  if (doStroke) {
    ctx.stroke();
  }
};

p5.Renderer2D.prototype.line = function(x1, y1, x2, y2) {
  var ctx = this.drawingContext;
  if (!this._doStroke) {
    return this;
  } else if(this._getStroke() === styleEmpty){
    return this;
  }
  // Translate the line by (0.5, 0.5) to draw it crisp
  if (ctx.lineWidth % 2 === 1) {
    ctx.translate(0.5, 0.5);
  }
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();
  if (ctx.lineWidth % 2 === 1) {
    ctx.translate(-0.5, -0.5);
  }
  return this;
};

p5.Renderer2D.prototype.point = function(x, y) {
  var ctx = this.drawingContext;
  if (!this._doStroke) {
    return this;
  } else if(this._getStroke() === styleEmpty){
    return this;
  }
  x = Math.round(x);
  y = Math.round(y);
  if (ctx.lineWidth > 1) {
    ctx.beginPath();
    ctx.arc(
      x,
      y,
      ctx.lineWidth / 2,
      0,
      constants.TWO_PI,
      false
    );
    ctx.fill();
  } else {
    ctx.fillRect(x, y, 1, 1);
  }
};

p5.Renderer2D.prototype.quad =
  function(x1, y1, x2, y2, x3, y3, x4, y4) {
  var ctx = this.drawingContext;
  var doFill = this._doFill, doStroke = this._doStroke;
  if (doFill && !doStroke) {
    if(this._getFill() === styleEmpty) {
      return this;
    }
  } else if (!doFill && doStroke) {
    if(this._getStroke() === styleEmpty) {
      return this;
    }
  }
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.lineTo(x3, y3);
  ctx.lineTo(x4, y4);
  ctx.closePath();
  if (doFill) {
    ctx.fill();
  }
  if (doStroke) {
    ctx.stroke();
  }
  return this;
};

p5.Renderer2D.prototype.rect = function(args) {
  var x = args[0],
    y = args[1],
    w = args[2],
    h = args[3],
    tl = args[4],
    tr = args[5],
    br = args[6],
    bl = args[7];
  var ctx = this.drawingContext;
  var doFill = this._doFill, doStroke = this._doStroke;
  if (doFill && !doStroke) {
    if(this._getFill() === styleEmpty) {
      return this;
    }
  } else if (!doFill && doStroke) {
    if(this._getStroke() === styleEmpty) {
      return this;
    }
  }
  // Translate the line by (0.5, 0.5) to draw a crisp rectangle border
  if (this._doStroke && ctx.lineWidth % 2 === 1) {
    ctx.translate(0.5, 0.5);
  }
  ctx.beginPath();

  if (typeof tl === 'undefined') {
    // No rounded corners
    ctx.rect(x, y, w, h);
  } else {
    // At least one rounded corner
    // Set defaults when not specified
    if (typeof tr === 'undefined') { tr = tl; }
    if (typeof br === 'undefined') { br = tr; }
    if (typeof bl === 'undefined') { bl = br; }

    var hw = w / 2;
    var hh = h / 2;

    // Clip radii
    if (w < 2 * tl) { tl = hw; }
    if (h < 2 * tl) { tl = hh; }
    if (w < 2 * tr) { tr = hw; }
    if (h < 2 * tr) { tr = hh; }
    if (w < 2 * br) { br = hw; }
    if (h < 2 * br) { br = hh; }
    if (w < 2 * bl) { bl = hw; }
    if (h < 2 * bl) { bl = hh; }

    // Draw shape
    ctx.beginPath();
    ctx.moveTo(x + tl, y);
    ctx.arcTo(x + w, y, x + w, y + h, tr);
    ctx.arcTo(x + w, y + h, x, y + h, br);
    ctx.arcTo(x, y + h, x, y, bl);
    ctx.arcTo(x, y, x + w, y, tl);
    ctx.closePath();
  }
  if (this._doFill) {
    ctx.fill();
  }
  if (this._doStroke) {
    ctx.stroke();
  }
  if (this._doStroke && ctx.lineWidth % 2 === 1) {
    ctx.translate(-0.5, -0.5);
  }
  return this;
};

p5.Renderer2D.prototype.triangle = function(args) {
  var ctx = this.drawingContext;
  var doFill = this._doFill, doStroke = this._doStroke;
  var x1=args[0], y1=args[1];
  var x2=args[2], y2=args[3];
  var x3=args[4], y3=args[5];
  if (doFill && !doStroke) {
    if(this._getFill() === styleEmpty) {
      return this;
    }
  } else if (!doFill && doStroke) {
    if(this._getStroke() === styleEmpty) {
      return this;
    }
  }
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.lineTo(x3, y3);
  ctx.closePath();
  if (doFill) {
    ctx.fill();
  }
  if (doStroke) {
    ctx.stroke();
  }
};

p5.Renderer2D.prototype.endShape =
function (mode, vertices, isCurve, isBezier,
    isQuadratic, isContour, shapeKind) {
  if (vertices.length === 0) {
    return this;
  }
  if (!this._doStroke && !this._doFill) {
    return this;
  }
  var closeShape = mode === constants.CLOSE;
  var v;
  if (closeShape && !isContour) {
    vertices.push(vertices[0]);
  }
  var i, j;
  var numVerts = vertices.length;
  if (isCurve && (shapeKind === constants.POLYGON || shapeKind === null)) {
    if (numVerts > 3) {
      var b = [], s = 1 - this._curveTightness;
      this.drawingContext.beginPath();
      this.drawingContext.moveTo(vertices[1][0], vertices[1][1]);
      for (i = 1; i + 2 < numVerts; i++) {
        v = vertices[i];
        b[0] = [
          v[0],
          v[1]
        ];
        b[1] = [
          v[0] + (s * vertices[i + 1][0] - s * vertices[i - 1][0]) / 6,
          v[1] + (s * vertices[i + 1][1] - s * vertices[i - 1][1]) / 6
        ];
        b[2] = [
          vertices[i + 1][0] +
          (s * vertices[i][0]-s * vertices[i + 2][0]) / 6,
          vertices[i + 1][1]+(s * vertices[i][1] - s*vertices[i + 2][1]) / 6
        ];
        b[3] = [
          vertices[i + 1][0],
          vertices[i + 1][1]
        ];
        this.drawingContext.bezierCurveTo(b[1][0],b[1][1],
          b[2][0],b[2][1],b[3][0],b[3][1]);
      }
      if (closeShape) {
        this.drawingContext.lineTo(vertices[i + 1][0], vertices[i + 1][1]);
      }
      this._doFillStrokeClose();
    }
  } else if (isBezier&&(shapeKind===constants.POLYGON ||shapeKind === null)) {
    this.drawingContext.beginPath();
    for (i = 0; i < numVerts; i++) {
      if (vertices[i].isVert) {
        if (vertices[i].moveTo) {
          this.drawingContext.moveTo(vertices[i][0], vertices[i][1]);
        } else {
          this.drawingContext.lineTo(vertices[i][0], vertices[i][1]);
        }
      } else {
        this.drawingContext.bezierCurveTo(vertices[i][0], vertices[i][1],
          vertices[i][2], vertices[i][3], vertices[i][4], vertices[i][5]);
      }
    }
    this._doFillStrokeClose();
  } else if (isQuadratic &&
    (shapeKind === constants.POLYGON || shapeKind === null)) {
    this.drawingContext.beginPath();
    for (i = 0; i < numVerts; i++) {
      if (vertices[i].isVert) {
        if (vertices[i].moveTo) {
          this.drawingContext.moveTo([0], vertices[i][1]);
        } else {
          this.drawingContext.lineTo(vertices[i][0], vertices[i][1]);
        }
      } else {
        this.drawingContext.quadraticCurveTo(vertices[i][0], vertices[i][1],
          vertices[i][2], vertices[i][3]);
      }
    }
    this._doFillStrokeClose();
  } else {
    if (shapeKind === constants.POINTS) {
      for (i = 0; i < numVerts; i++) {
        v = vertices[i];
        if (this._doStroke) {
          this._pInst.stroke(v[6]);
        }
        this._pInst.point(v[0], v[1]);
      }
    } else if (shapeKind === constants.LINES) {
      for (i = 0; i + 1 < numVerts; i += 2) {
        v = vertices[i];
        if (this._doStroke) {
          this._pInst.stroke(vertices[i + 1][6]);
        }
        this._pInst.line(v[0], v[1], vertices[i + 1][0], vertices[i + 1][1]);
      }
    } else if (shapeKind === constants.TRIANGLES) {
      for (i = 0; i + 2 < numVerts; i += 3) {
        v = vertices[i];
        this.drawingContext.beginPath();
        this.drawingContext.moveTo(v[0], v[1]);
        this.drawingContext.lineTo(vertices[i + 1][0], vertices[i + 1][1]);
        this.drawingContext.lineTo(vertices[i + 2][0], vertices[i + 2][1]);
        this.drawingContext.lineTo(v[0], v[1]);
        if (this._doFill) {
          this._pInst.fill(vertices[i + 2][5]);
          this.drawingContext.fill();
        }
        if (this._doStroke) {
          this._pInst.stroke(vertices[i + 2][6]);
          this.drawingContext.stroke();
        }
        this.drawingContext.closePath();
      }
    } else if (shapeKind === constants.TRIANGLE_STRIP) {
      for (i = 0; i + 1 < numVerts; i++) {
        v = vertices[i];
        this.drawingContext.beginPath();
        this.drawingContext.moveTo(vertices[i + 1][0], vertices[i + 1][1]);
        this.drawingContext.lineTo(v[0], v[1]);
        if (this._doStroke) {
          this._pInst.stroke(vertices[i + 1][6]);
        }
        if (this._doFill) {
          this._pInst.fill(vertices[i + 1][5]);
        }
        if (i + 2 < numVerts) {
          this.drawingContext.lineTo(vertices[i + 2][0], vertices[i + 2][1]);
          if (this._doStroke) {
            this._pInst.stroke(vertices[i + 2][6]);
          }
          if (this._doFill) {
            this._pInst.fill(vertices[i + 2][5]);
          }
        }
        this._doFillStrokeClose();
      }
    } else if (shapeKind === constants.TRIANGLE_FAN) {
      if (numVerts > 2) {
        // For performance reasons, try to batch as many of the
        // fill and stroke calls as possible.
        this.drawingContext.beginPath();
        for (i = 2; i < numVerts; i++) {
          v = vertices[i];
          this.drawingContext.moveTo(vertices[0][0], vertices[0][1]);
          this.drawingContext.lineTo(vertices[i - 1][0], vertices[i - 1][1]);
          this.drawingContext.lineTo(v[0], v[1]);
          this.drawingContext.lineTo(vertices[0][0], vertices[0][1]);
          // If the next colour is going to be different, stroke / fill now
          if (i < numVerts - 1) {
            if ( (this._doFill && v[5] !== vertices[i + 1][5]) ||
                 (this._doStroke && v[6] !== vertices[i + 1][6])) {
              if (this._doFill) {
                this._pInst.fill(v[5]);
                this.drawingContext.fill();
                this._pInst.fill(vertices[i + 1][5]);
              }
              if (this._doStroke) {
                this._pInst.stroke(v[6]);
                this.drawingContext.stroke();
                this._pInst.stroke(vertices[i + 1][6]);
              }
              this.drawingContext.closePath();
              this.drawingContext.beginPath(); // Begin the next one
            }
          }
        }
        this._doFillStrokeClose();
      }
    } else if (shapeKind === constants.QUADS) {
      for (i = 0; i + 3 < numVerts; i += 4) {
        v = vertices[i];
        this.drawingContext.beginPath();
        this.drawingContext.moveTo(v[0], v[1]);
        for (j = 1; j < 4; j++) {
          this.drawingContext.lineTo(vertices[i + j][0], vertices[i + j][1]);
        }
        this.drawingContext.lineTo(v[0], v[1]);
        if (this._doFill) {
          this._pInst.fill(vertices[i + 3][5]);
        }
        if (this._doStroke) {
          this._pInst.stroke(vertices[i + 3][6]);
        }
        this._doFillStrokeClose();
      }
    } else if (shapeKind === constants.QUAD_STRIP) {
      if (numVerts > 3) {
        for (i = 0; i + 1 < numVerts; i += 2) {
          v = vertices[i];
          this.drawingContext.beginPath();
          if (i + 3 < numVerts) {
            this.drawingContext.moveTo(vertices[i + 2][0], vertices[i+2][1]);
            this.drawingContext.lineTo(v[0], v[1]);
            this.drawingContext.lineTo(vertices[i + 1][0], vertices[i+1][1]);
            this.drawingContext.lineTo(vertices[i + 3][0], vertices[i+3][1]);
            if (this._doFill) {
              this._pInst.fill(vertices[i + 3][5]);
            }
            if (this._doStroke) {
              this._pInst.stroke(vertices[i + 3][6]);
            }
          } else {
            this.drawingContext.moveTo(v[0], v[1]);
            this.drawingContext.lineTo(vertices[i + 1][0], vertices[i+1][1]);
          }
          this._doFillStrokeClose();
        }
      }
    } else {
      this.drawingContext.beginPath();
      this.drawingContext.moveTo(vertices[0][0], vertices[0][1]);
      for (i = 1; i < numVerts; i++) {
        v = vertices[i];
        if (v.isVert) {
          if (v.moveTo) {
            this.drawingContext.moveTo(v[0], v[1]);
          } else {
            this.drawingContext.lineTo(v[0], v[1]);
          }
        }
      }
      this._doFillStrokeClose();
    }
  }
  isCurve = false;
  isBezier = false;
  isQuadratic = false;
  isContour = false;
  if (closeShape) {
    vertices.pop();
  }
  return this;
};
//////////////////////////////////////////////
// SHAPE | Attributes
//////////////////////////////////////////////

p5.Renderer2D.prototype.noSmooth = function() {
  if ('imageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.imageSmoothingEnabled = false;
  }
  else if ('mozImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.mozImageSmoothingEnabled = false;
  }
  else if ('webkitImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.webkitImageSmoothingEnabled = false;
  }
  else if ('msImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.msImageSmoothingEnabled = false;
  }
  return this;
};

p5.Renderer2D.prototype.smooth = function() {
  if ('imageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.imageSmoothingEnabled = true;
  }
  else if ('mozImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.mozImageSmoothingEnabled = true;
  }
  else if ('webkitImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.webkitImageSmoothingEnabled = true;
  }
  else if ('msImageSmoothingEnabled' in this.drawingContext) {
    this.drawingContext.msImageSmoothingEnabled = true;
  }
  return this;
};

p5.Renderer2D.prototype.strokeCap = function(cap) {
  if (cap === constants.ROUND ||
    cap === constants.SQUARE ||
    cap === constants.PROJECT) {
    this.drawingContext.lineCap = cap;
  }
  return this;
};

p5.Renderer2D.prototype.strokeJoin = function(join) {
  if (join === constants.ROUND ||
    join === constants.BEVEL ||
    join === constants.MITER) {
    this.drawingContext.lineJoin = join;
  }
  return this;
};

p5.Renderer2D.prototype.strokeWeight = function(w) {
  if (typeof w === 'undefined' || w === 0) {
    // hack because lineWidth 0 doesn't work
    this.drawingContext.lineWidth = 0.0001;
  } else {
    this.drawingContext.lineWidth = w;
  }
  return this;
};

p5.Renderer2D.prototype._getFill = function(){
  return this._cachedFillStyle;
};

p5.Renderer2D.prototype._setFill = function(fillStyle){
  if (fillStyle !== this._cachedFillStyle) {
    this.drawingContext.fillStyle = fillStyle;
    this._cachedFillStyle = fillStyle;
  }
};

p5.Renderer2D.prototype._getStroke = function(){
  return this._cachedStrokeStyle;
};

p5.Renderer2D.prototype._setStroke = function(strokeStyle){
  if (strokeStyle !== this._cachedStrokeStyle) {
    this.drawingContext.strokeStyle = strokeStyle;
    this._cachedStrokeStyle = strokeStyle;
  }
};



//////////////////////////////////////////////
// SHAPE | Curves
//////////////////////////////////////////////
p5.Renderer2D.prototype.bezier = function (x1, y1, x2, y2, x3, y3, x4, y4) {
  this._pInst.beginShape();
  this._pInst.vertex(x1, y1);
  this._pInst.bezierVertex(x2, y2, x3, y3, x4, y4);
  this._pInst.endShape();
  return this;
};

p5.Renderer2D.prototype.curve = function (x1, y1, x2, y2, x3, y3, x4, y4) {
  this._pInst.beginShape();
  this._pInst.curveVertex(x1, y1);
  this._pInst.curveVertex(x2, y2);
  this._pInst.curveVertex(x3, y3);
  this._pInst.curveVertex(x4, y4);
  this._pInst.endShape();
  return this;
};

//////////////////////////////////////////////
// SHAPE | Vertex
//////////////////////////////////////////////

p5.Renderer2D.prototype._doFillStrokeClose = function () {
  if (this._doFill) {
    this.drawingContext.fill();
  }
  if (this._doStroke) {
    this.drawingContext.stroke();
  }
  this.drawingContext.closePath();
};

//////////////////////////////////////////////
// TRANSFORM
//////////////////////////////////////////////

p5.Renderer2D.prototype.applyMatrix =
function(n00, n01, n02, n10, n11, n12) {
  this.drawingContext.transform(n00, n01, n02, n10, n11, n12);
};

p5.Renderer2D.prototype.resetMatrix = function() {
  this.drawingContext.setTransform(1, 0, 0, 1, 0, 0);
  this.drawingContext.scale(this._pInst._pixelDensity,
                            this._pInst._pixelDensity);
  return this;
};

p5.Renderer2D.prototype.rotate = function(r) {
  this.drawingContext.rotate(r);
};

p5.Renderer2D.prototype.scale = function(x,y) {
  this.drawingContext.scale(x, y);
  return this;
};

p5.Renderer2D.prototype.shearX = function(angle) {
  if (this._pInst._angleMode === constants.DEGREES) {
    // undoing here, because it gets redone in tan()
    angle = this._pInst.degrees(angle);
  }
  this.drawingContext.transform(1, 0, this._pInst.tan(angle), 1, 0, 0);
  return this;
};

p5.Renderer2D.prototype.shearY = function(angle) {
  if (this._pInst._angleMode === constants.DEGREES) {
    // undoing here, because it gets redone in tan()
    angle = this._pInst.degrees(angle);
  }
  this.drawingContext.transform(1, this._pInst.tan(angle), 0, 1, 0, 0);
  return this;
};

p5.Renderer2D.prototype.translate = function(x, y) {
  this.drawingContext.translate(x, y);
  return this;
};

//////////////////////////////////////////////
// TYPOGRAPHY
//
//////////////////////////////////////////////

p5.Renderer2D.prototype.text = function (str, x, y, maxWidth, maxHeight) {

  var p = this._pInst, cars, n, ii, jj, line, testLine,
    testWidth, words, totalHeight, baselineHacked,
    finalMaxHeight = Number.MAX_VALUE;

  // baselineHacked: (HACK)
  // A temporary fix to conform to Processing's implementation
  // of BASELINE vertical alignment in a bounding box

  if (!(this._doFill || this._doStroke)) {
    return;
  }

  if (typeof str !== 'string') {
    str = str.toString();
  }

  str = str.replace(/(\t)/g, '  ');
  cars = str.split('\n');

  if (typeof maxWidth !== 'undefined') {

    totalHeight = 0;
    for (ii = 0; ii < cars.length; ii++) {
      line = '';
      words = cars[ii].split(' ');
      for (n = 0; n < words.length; n++) {
        testLine = line + words[n] + ' ';
        testWidth = this.textWidth(testLine);
        if (testWidth > maxWidth) {
          line = words[n] + ' ';
          totalHeight += p.textLeading();
        } else {
          line = testLine;
        }
      }
    }

    if (this._rectMode === constants.CENTER) {

      x -= maxWidth / 2;
      y -= maxHeight / 2;
    }

    switch (this.drawingContext.textAlign) {

      case constants.CENTER:
        x += maxWidth / 2;
        break;
      case constants.RIGHT:
        x += maxWidth;
        break;
    }

    if (typeof maxHeight !== 'undefined') {

      switch (this.drawingContext.textBaseline) {
        case constants.BOTTOM:
          y += (maxHeight - totalHeight);
          break;
        case constants._CTX_MIDDLE: // CENTER?
          y += (maxHeight - totalHeight) / 2;
          break;
        case constants.BASELINE:
          baselineHacked = true;
          this.drawingContext.textBaseline = constants.TOP;
          break;
      }

      // remember the max-allowed y-position for any line (fix to #928)
      finalMaxHeight = (y + maxHeight) - p.textAscent();
    }

    for (ii = 0; ii < cars.length; ii++) {

      line = '';
      words = cars[ii].split(' ');
      for (n = 0; n < words.length; n++) {
        testLine = line + words[n] + ' ';
        testWidth = this.textWidth(testLine);
        if (testWidth > maxWidth && line.length > 0) {
          this._renderText(p, line, x, y, finalMaxHeight);
          line = words[n] + ' ';
          y += p.textLeading();
        } else {
          line = testLine;
        }
      }

      this._renderText(p, line, x, y, finalMaxHeight);
      y += p.textLeading();
    }
  }
  else {
    // Offset to account for vertically centering multiple lines of text - no
    // need to adjust anything for vertical align top or baseline
    var offset = 0,
      vAlign = p.textAlign().vertical;
    if (vAlign === constants.CENTER) {
      offset = ((cars.length - 1) * p.textLeading()) / 2;
    } else if (vAlign === constants.BOTTOM) {
      offset = (cars.length - 1) * p.textLeading();
    }

    for (jj = 0; jj < cars.length; jj++) {

      this._renderText(p, cars[jj], x, y-offset, finalMaxHeight);
      y += p.textLeading();
    }
  }

  if (baselineHacked) {
    this.drawingContext.textBaseline = constants.BASELINE;
  }

  return p;
};

p5.Renderer2D.prototype._renderText = function(p, line, x, y, maxY) {

  if (y >= maxY) {
    return; // don't render lines beyond our maxY position
  }

  p.push(); // fix to #803

  if (!this._isOpenType()) {  // a system/browser font

    // no stroke unless specified by user
    if (this._doStroke && this._strokeSet) {

      this.drawingContext.strokeText(line, x, y);
    }

    if (this._doFill) {

      // if fill hasn't been set by user, use default text fill
      if (! this._fillSet) {
        this._setFill(constants._DEFAULT_TEXT_FILL);
      }

      this.drawingContext.fillText(line, x, y);
    }
  }
  else { // an opentype font, let it handle the rendering

    this._textFont._renderPath(line, x, y, { renderer: this });
  }

  p.pop();

  return p;
};

p5.Renderer2D.prototype.textWidth = function(s) {

  if (this._isOpenType()) {

    return this._textFont._textWidth(s, this._textSize);
  }

  return this.drawingContext.measureText(s).width;
};

p5.Renderer2D.prototype.textAlign = function(h, v) {

  if (arguments.length) {

    if (h === constants.LEFT ||
      h === constants.RIGHT ||
      h === constants.CENTER) {

      this.drawingContext.textAlign = h;
    }

    if (v === constants.TOP ||
      v === constants.BOTTOM ||
      v === constants.CENTER ||
      v === constants.BASELINE) {

      if (v === constants.CENTER) {
        this.drawingContext.textBaseline = constants._CTX_MIDDLE;
      } else {
        this.drawingContext.textBaseline = v;
      }
    }

    return this._pInst;

  } else {

    var valign = this.drawingContext.textBaseline;

    if (valign === constants._CTX_MIDDLE) {

      valign = constants.CENTER;
    }

    return {

      horizontal: this.drawingContext.textAlign,
      vertical: valign
    };
  }
};

p5.Renderer2D.prototype._applyTextProperties = function() {

  var font, p = this._pInst;

  this._setProperty('_textAscent', null);
  this._setProperty('_textDescent', null);

  font = this._textFont;

  if (this._isOpenType()) {

    font = this._textFont.font.familyName;
    this._setProperty('_textStyle', this._textFont.font.styleName);
  }

  this.drawingContext.font = (this._textStyle || 'normal') +
    ' ' + (this._textSize || 12) + 'px ' + (font || 'sans-serif');

  return p;
};


//////////////////////////////////////////////
// STRUCTURE
//////////////////////////////////////////////

p5.Renderer2D.prototype.push = function() {
  this.drawingContext.save();
};

p5.Renderer2D.prototype.pop = function() {
  this.drawingContext.restore();
  // Re-cache the fill / stroke state
  this._cachedFillStyle = this.drawingContext.fillStyle;
  this._cachedStrokeStyle = this.drawingContext.strokeStyle;
};

module.exports = p5.Renderer2D;

},{"../image/filters":73,"./canvas":52,"./constants":53,"./core":54,"./p5.Renderer":61}],63:[function(_dereq_,module,exports){
/**
 * @module Rendering
 * @submodule Rendering
 * @for p5
 */

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');
_dereq_('./p5.Graphics');
_dereq_('./p5.Renderer2D');
_dereq_('../webgl/p5.RendererGL');
var defaultId = 'defaultCanvas0'; // this gets set again in createCanvas

/**
 * Creates a canvas element in the document, and sets the dimensions of it
 * in pixels. This method should be called only once at the start of setup.
 * Calling createCanvas more than once in a sketch will result in very
 * unpredicable behavior. If you want more than one drawing canvas
 * you could use createGraphics (hidden by default but it can be shown).
 * <br><br>
 * The system variables width and height are set by the parameters passed
 * to this function. If createCanvas() is not used, the window will be
 * given a default size of 100x100 pixels.
 * <br><br>
 * For more ways to position the canvas, see the
 * <a href='https://github.com/processing/p5.js/wiki/Positioning-your-canvas'>
 * positioning the canvas</a> wiki page.
 *
 * @method createCanvas
 * @param  {Number} w width of the canvas
 * @param  {Number} h height of the canvas
 * @param  {Constant} [renderer] either P2D or WEBGL
 * @return {HTMLCanvasElement} canvas generated
 * @example
 * <div>
 * <code>
 * function setup() {
 *   createCanvas(100, 50);
 *   background(153);
 *   line(0, 0, width, height);
 * }
 * </code>
 * </div>
 *
 * @alt
 * Black line extending from top-left of canvas to bottom right.
 *
 */

p5.prototype.createCanvas = function(w, h, renderer) {
  //optional: renderer, otherwise defaults to p2d
  var r = renderer || constants.P2D;
  var isDefault, c;

  //4th arg (isDefault) used when called onLoad,
  //otherwise hidden to the public api
  if(arguments[3]){
    isDefault =
    (typeof arguments[3] === 'boolean') ? arguments[3] : false;
  }

  if(r === constants.WEBGL){
    c = document.getElementById(defaultId);
    if(c){ //if defaultCanvas already exists
      c.parentNode.removeChild(c); //replace the existing defaultCanvas
    }
    c = document.createElement('canvas');
    c.id = defaultId;
  }
  else {
    if (isDefault) {
      c = document.createElement('canvas');
      var i = 0;
      while (document.getElementById('defaultCanvas'+i)) {
        i++;
      }
      defaultId = 'defaultCanvas'+i;
      c.id = defaultId;
    } else { // resize the default canvas if new one is created
      c = this.canvas;
    }
  }

  // set to invisible if still in setup (to prevent flashing with manipulate)
  if (!this._setupDone) {
    c.dataset.hidden = true; // tag to show later
    c.style.visibility='hidden';
  }

  if (this._userNode) { // user input node case
    this._userNode.appendChild(c);
  } else {
    document.body.appendChild(c);
  }



  // Init our graphics renderer
  //webgl mode
  if (r === constants.WEBGL) {
    this._setProperty('_renderer', new p5.RendererGL(c, this, true));
    this._isdefaultGraphics = true;
  }
  //P2D mode
  else {
    if (!this._isdefaultGraphics) {
      this._setProperty('_renderer', new p5.Renderer2D(c, this, true));
      this._isdefaultGraphics = true;
    }
  }
  this._renderer.resize(w, h);
  this._renderer._applyDefaults();
  if (isDefault) { // only push once
    this._elements.push(this._renderer);
  }
  return this._renderer;
};

/**
 * Resizes the canvas to given width and height. The canvas will be cleared
 * and draw will be called immediately, allowing the sketch to re-render itself
 * in the resized canvas.
 * @method resizeCanvas
 * @param  {Number} w width of the canvas
 * @param  {Number} h height of the canvas
 * @param  {Boolean} noRedraw don't redraw the canvas immediately
 * @example
 * <div class="norender"><code>
 * function setup() {
 *   createCanvas(windowWidth, windowHeight);
 * }
 *
 * function draw() {
 *  background(0, 100, 200);
 * }
 *
 * function windowResized() {
 *   resizeCanvas(windowWidth, windowHeight);
 * }
 * </code></div>
 *
 * @alt
 * No image displayed.
 *
 */
p5.prototype.resizeCanvas = function (w, h, noRedraw) {
  if (this._renderer) {

    // save canvas properties
    var props = {};
    for (var key in this.drawingContext) {
      var val = this.drawingContext[key];
      if (typeof val !== 'object' && typeof val !== 'function') {
        props[key] = val;
      }
    }
    this._renderer.resize(w, h);
    // reset canvas properties
    for (var savedKey in props) {
      this.drawingContext[savedKey] = props[savedKey];
    }
    if (!noRedraw) {
      this.redraw();
    }
  }
};


/**
 * Removes the default canvas for a p5 sketch that doesn't
 * require a canvas
 * @method noCanvas
 * @example
 * <div>
 * <code>
 * function setup() {
 *   noCanvas();
 * }
 * </code>
 * </div>
 *
 * @alt
 * no image displayed
 *
 */
p5.prototype.noCanvas = function() {
  if (this.canvas) {
    this.canvas.parentNode.removeChild(this.canvas);
  }
};

/**
 * Creates and returns a new p5.Renderer object. Use this class if you need
 * to draw into an off-screen graphics buffer. The two parameters define the
 * width and height in pixels.
 *
 * @method createGraphics
 * @param  {Number} w width of the offscreen graphics buffer
 * @param  {Number} h height of the offscreen graphics buffer
 * @param  {Constant} [renderer] either P2D or WEBGL
 * undefined defaults to p2d
 * @return {p5.Graphics} offscreen graphics buffer
 * @example
 * <div>
 * <code>
 * var pg;
 * function setup() {
 *   createCanvas(100, 100);
 *   pg = createGraphics(100, 100);
 * }
 * function draw() {
 *   background(200);
 *   pg.background(100);
 *   pg.noStroke();
 *   pg.ellipse(pg.width/2, pg.height/2, 50, 50);
 *   image(pg, 50, 50);
 *   image(pg, 0, 0, 50, 50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 4 grey squares alternating light and dark grey. White quarter circle mid-left.
 *
 */
p5.prototype.createGraphics = function(w, h, renderer){
  return new p5.Graphics(w, h, renderer, this);
};

/**
 * Blends the pixels in the display window according to the defined mode.
 * There is a choice of the following modes to blend the source pixels (A)
 * with the ones of pixels already in the display window (B):
 * <ul>
 * <li><code>BLEND</code> - linear interpolation of colours: C =
 * A*factor + B. This is the default blending mode.</li>
 * <li><code>ADD</code> - sum of A and B</li>
 * <li><code>DARKEST</code> - only the darkest colour succeeds: C =
 * min(A*factor, B).</li>
 * <li><code>LIGHTEST</code> - only the lightest colour succeeds: C =
 * max(A*factor, B).</li>
 * <li><code>DIFFERENCE</code> - subtract colors from underlying image.</li>
 * <li><code>EXCLUSION</code> - similar to <code>DIFFERENCE</code>, but less
 * extreme.</li>
 * <li><code>MULTIPLY</code> - multiply the colors, result will always be
 * darker.</li>
 * <li><code>SCREEN</code> - opposite multiply, uses inverse values of the
 * colors.</li>
 * <li><code>REPLACE</code> - the pixels entirely replace the others and
 * don't utilize alpha (transparency) values.</li>
 * <li><code>OVERLAY</code> - mix of <code>MULTIPLY</code> and <code>SCREEN
 * </code>. Multiplies dark values, and screens light values.</li>
 * <li><code>HARD_LIGHT</code> - <code>SCREEN</code> when greater than 50%
 * gray, <code>MULTIPLY</code> when lower.</li>
 * <li><code>SOFT_LIGHT</code> - mix of <code>DARKEST</code> and
 * <code>LIGHTEST</code>. Works like <code>OVERLAY</code>, but not as harsh.
 * </li>
 * <li><code>DODGE</code> - lightens light tones and increases contrast,
 * ignores darks.</li>
 * <li><code>BURN</code> - darker areas are applied, increasing contrast,
 * ignores lights.</li>
 * </ul>
 *
 * @method blendMode
 * @param  {Constant} mode blend mode to set for canvas.
 *                either BLEND, DARKEST, LIGHTEST, DIFFERENCE, MULTIPLY,
 *                EXCLUSION, SCREEN, REPLACE, OVERLAY, HARD_LIGHT,
 *                SOFT_LIGHT, DODGE, BURN, ADD or NORMAL
 * @example
 * <div>
 * <code>
 * blendMode(LIGHTEST);
 * strokeWeight(30);
 * stroke(80, 150, 255);
 * line(25, 25, 75, 75);
 * stroke(255, 50, 50);
 * line(75, 25, 25, 75);
 * </code>
 * </div>
 * <div>
 * <code>
 * blendMode(MULTIPLY);
 * strokeWeight(30);
 * stroke(80, 150, 255);
 * line(25, 25, 75, 75);
 * stroke(255, 50, 50);
 * line(75, 25, 25, 75);
 * </code>
 * </div>
 * @alt
 * translucent image thick red & blue diagonal rounded lines intersecting center
 * Thick red & blue diagonal rounded lines intersecting center. dark at overlap
 *
 */
p5.prototype.blendMode = function(mode) {
  if (mode === constants.BLEND || mode === constants.DARKEST ||
    mode === constants.LIGHTEST || mode === constants.DIFFERENCE ||
    mode === constants.MULTIPLY || mode === constants.EXCLUSION ||
    mode === constants.SCREEN || mode === constants.REPLACE ||
    mode === constants.OVERLAY || mode === constants.HARD_LIGHT ||
    mode === constants.SOFT_LIGHT || mode === constants.DODGE ||
    mode === constants.BURN || mode === constants.ADD ||
    mode === constants.NORMAL) {
    this._renderer.blendMode(mode);
  } else {
    throw new Error('Mode '+mode+' not recognized.');
  }
};

module.exports = p5;

},{"../webgl/p5.RendererGL":105,"./constants":53,"./core":54,"./p5.Graphics":60,"./p5.Renderer2D":62}],64:[function(_dereq_,module,exports){

// requestAnim shim layer by Paul Irish
window.requestAnimationFrame = (function(){
  return window.requestAnimationFrame      ||
        window.webkitRequestAnimationFrame ||
        window.mozRequestAnimationFrame    ||
        window.oRequestAnimationFrame      ||
        window.msRequestAnimationFrame     ||
        function(callback, element){
          // should '60' here be framerate?
          window.setTimeout(callback, 1000 / 60);
        };
})();

// use window.performance() to get max fast and accurate time in milliseconds
window.performance = window.performance || {};
window.performance.now = (function(){
  var load_date = Date.now();
  return window.performance.now        ||
        window.performance.mozNow      ||
        window.performance.msNow       ||
        window.performance.oNow        ||
        window.performance.webkitNow   ||
        function () {
          return Date.now() - load_date;
        };
})();

/*
// http://paulirish.com/2011/requestanimationframe-for-smart-animating/
// http://my.opera.com/emoller/blog/2011/12/20/
// requestanimationframe-for-smart-er-animating
// requestAnimationFrame polyfill by Erik Möller
// fixes from Paul Irish and Tino Zijdel
(function() {
  var lastTime = 0;
  var vendors = ['ms', 'moz', 'webkit', 'o'];
  for (var x = 0; x < vendors.length && !window.requestAnimationFrame; ++x) {
    window.requestAnimationFrame =
      window[vendors[x]+'RequestAnimationFrame'];
    window.cancelAnimationFrame =
      window[vendors[x]+'CancelAnimationFrame'] ||
      window[vendors[x]+'CancelRequestAnimationFrame'];
  }

  if (!window.requestAnimationFrame) {
    window.requestAnimationFrame = function(callback, element) {
      var currTime = new Date().getTime();
      var timeToCall = Math.max(0, 16 - (currTime - lastTime));
      var id = window.setTimeout(function()
        { callback(currTime + timeToCall); }, timeToCall);
      lastTime = currTime + timeToCall;
      return id;
    };
  }

  if (!window.cancelAnimationFrame) {
    window.cancelAnimationFrame = function(id) {
      clearTimeout(id);
    };
  }
}());
*/

/**
 * shim for Uint8ClampedArray.slice
 * (allows arrayCopy to work with pixels[])
 * with thanks to http://halfpapstudios.com/blog/tag/html5-canvas/
 * Enumerable set to false to protect for...in from
 * Uint8ClampedArray.prototype pollution.
 */
(function () {
  'use strict';
  if (typeof Uint8ClampedArray !== 'undefined' &&
      !Uint8ClampedArray.prototype.slice) {
    Object.defineProperty(Uint8ClampedArray.prototype, 'slice', {
      value: Array.prototype.slice,
      writable: true, configurable: true, enumerable: false
    });
  }
}());

},{}],65:[function(_dereq_,module,exports){
/**
 * @module Structure
 * @submodule Structure
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('./core');

p5.prototype.exit = function() {
  throw 'exit() not implemented, see remove()';
};
/**
 * Stops p5.js from continuously executing the code within draw().
 * If loop() is called, the code in draw() begins to run continuously again.
 * If using noLoop() in setup(), it should be the last line inside the block.
 * <br><br>
 * When noLoop() is used, it's not possible to manipulate or access the
 * screen inside event handling functions such as mousePressed() or
 * keyPressed(). Instead, use those functions to call redraw() or loop(),
 * which will run draw(), which can update the screen properly. This means
 * that when noLoop() has been called, no drawing can happen, and functions
 * like saveFrame() or loadPixels() may not be used.
 * <br><br>
 * Note that if the sketch is resized, redraw() will be called to update
 * the sketch, even after noLoop() has been specified. Otherwise, the sketch
 * would enter an odd state until loop() was called.
 *
 * @method noLoop
 * @example
 * <div><code>
 * function setup() {
 *   createCanvas(100, 100);
 *   background(200);
 *   noLoop();
 * }

 * function draw() {
 *   line(10, 10, 90, 90);
 * }
 * </code></div>
 *
 * <div><code>
 * var x = 0;
 * function setup() {
 *   createCanvas(100, 100);
 * }
 *
 * function draw() {
 *   background(204);
 *   x = x + 0.1;
 *   if (x > width) {
 *     x = 0;
 *   }
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   noLoop();
 * }
 *
 * function mouseReleased() {
 *   loop();
 * }
 * </code></div>
 *
 * @alt
 * 113 pixel long line extending from top-left to bottom right of canvas.
 * horizontal line moves slowly from left. Loops but stops on mouse press.
 *
 */
p5.prototype.noLoop = function() {
  this._loop = false;
};
/**
 * By default, p5.js loops through draw() continuously, executing the code
 * within it. However, the draw() loop may be stopped by calling noLoop().
 * In that case, the draw() loop can be resumed with loop().
 *
 * @method loop
 * @example
 * <div><code>
 * var x = 0;
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   x = x + 0.1;
 *   if (x > width) {
 *     x = 0;
 *   }
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   loop();
 * }
 *
 * function mouseReleased() {
 *   noLoop();
 * }
 * </code></div>
 *
 * @alt
 * horizontal line moves slowly from left. Loops but stops on mouse press.
 *
 */

p5.prototype.loop = function() {
  this._loop = true;
  this._draw();
};

/**
 * The push() function saves the current drawing style settings and
 * transformations, while pop() restores these settings. Note that these
 * functions are always used together. They allow you to change the style
 * and transformation settings and later return to what you had. When a new
 * state is started with push(), it builds on the current style and transform
 * information. The push() and pop() functions can be embedded to provide
 * more control. (See the second example for a demonstration.)
 * <br><br>
 * push() stores information related to the current transformation state
 * and style settings controlled by the following functions: fill(),
 * stroke(), tint(), strokeWeight(), strokeCap(), strokeJoin(),
 * imageMode(), rectMode(), ellipseMode(), colorMode(), textAlign(),
 * textFont(), textMode(), textSize(), textLeading().
 *
 * @method push
 * @example
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33);  // Left circle
 *
 * push();  // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * translate(50, 0);
 * ellipse(0, 50, 33, 33);  // Middle circle
 * pop();  // Restore original state
 *
 * ellipse(100, 50, 33, 33);  // Right circle
 * </code>
 * </div>
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33);  // Left circle
 *
 * push();  // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(33, 50, 33, 33);  // Left-middle circle
 *
 * push();  // Start another new drawing state
 * stroke(0, 102, 153);
 * ellipse(66, 50, 33, 33);  // Right-middle circle
 * pop();  // Restore previous state
 *
 * pop();  // Restore original state
 *
 * ellipse(100, 50, 33, 33);  // Right circle
 * </code>
 * </div>
 *
 * @alt
 * Gold ellipse + thick black outline @center 2 white ellipses on left and right.
 * 2 Gold ellipses left black right blue stroke. 2 white ellipses on left+right.
 *
 */
p5.prototype.push = function () {
  this._renderer.push();
  this._styles.push({
    _doStroke: this._renderer._doStroke,
    _strokeSet: this._renderer._strokeSet,
    _doFill: this._renderer._doFill,
    _fillSet: this._renderer._fillSet,
    _tint: this._renderer._tint,
    _imageMode: this._renderer._imageMode,
    _rectMode: this._renderer._rectMode,
    _ellipseMode: this._renderer._ellipseMode,
    _colorMode: this._renderer._colorMode,
    _textFont: this._renderer._textFont,
    _textLeading: this._renderer._textLeading,
    _textSize: this._renderer._textSize,
    _textStyle: this._renderer._textStyle
  });
};

/**
 * The push() function saves the current drawing style settings and
 * transformations, while pop() restores these settings. Note that these
 * functions are always used together. They allow you to change the style
 * and transformation settings and later return to what you had. When a new
 * state is started with push(), it builds on the current style and transform
 * information. The push() and pop() functions can be embedded to provide
 * more control. (See the second example for a demonstration.)
 * <br><br>
 * push() stores information related to the current transformation state
 * and style settings controlled by the following functions: fill(),
 * stroke(), tint(), strokeWeight(), strokeCap(), strokeJoin(),
 * imageMode(), rectMode(), ellipseMode(), colorMode(), textAlign(),
 * textFont(), textMode(), textSize(), textLeading().
 *
 * @method pop
 * @example
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33);  // Left circle
 *
 * push();  // Start a new drawing state
 * translate(50, 0);
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(0, 50, 33, 33);  // Middle circle
 * pop();  // Restore original state
 *
 * ellipse(100, 50, 33, 33);  // Right circle
 * </code>
 * </div>
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33);  // Left circle
 *
 * push();  // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(33, 50, 33, 33);  // Left-middle circle
 *
 * push();  // Start another new drawing state
 * stroke(0, 102, 153);
 * ellipse(66, 50, 33, 33);  // Right-middle circle
 * pop();  // Restore previous state
 *
 * pop();  // Restore original state
 *
 * ellipse(100, 50, 33, 33);  // Right circle
 * </code>
 * </div>
 *
 * @alt
 * Gold ellipse + thick black outline @center 2 white ellipses on left and right.
 * 2 Gold ellipses left black right blue stroke. 2 white ellipses on left+right.
 *
 */
p5.prototype.pop = function () {
  this._renderer.pop();
  var lastS = this._styles.pop();
  for(var prop in lastS){
    this._renderer[prop] = lastS[prop];
  }
};

p5.prototype.pushStyle = function() {
  throw new Error('pushStyle() not used, see push()');
};

p5.prototype.popStyle = function() {
  throw new Error('popStyle() not used, see pop()');
};

/**
 *
 * Executes the code within draw() one time. This functions allows the
 * program to update the display window only when necessary, for example
 * when an event registered by mousePressed() or keyPressed() occurs.
 * <br><br>
 * In structuring a program, it only makes sense to call redraw() within
 * events such as mousePressed(). This is because redraw() does not run
 * draw() immediately (it only sets a flag that indicates an update is
 * needed).
 * <br><br>
 * The redraw() function does not work properly when called inside draw().
 * To enable/disable animations, use loop() and noLoop().
 * <br><br>
 * In addition you can set the number of redraws per method call. Just
 * add an integer as single parameter for the number of redraws.
 *
 * @method redraw
 * @param  {Integer} [n] Redraw for n-times. The default value is 1.
 * @example
 * <div><code>
 * var x = 0;
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   x += 1;
 *   redraw();
 * }
 * </code></div>
 *
 * <div class='norender'><code>
 * var x = 0;
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   x += 1;
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   redraw(5);
 * }
 * </code></div>
 *
 * @alt
 * black line on far left of canvas
 * black line on far left of canvas
 *
 */
p5.prototype.redraw = function () {
  this.resetMatrix();
  if(this._renderer.isP3D){
    this._renderer._update();
  }

  var numberOfRedraws = 1;
  if (arguments.length === 1) {
    try {
      if (parseInt(arguments[0]) > 1) {
        numberOfRedraws = parseInt(arguments[0]);
      }
    } catch (error) {
      // Do nothing, because the default value didn't be changed.
    }
  }
  var userSetup = this.setup || window.setup;
  var userDraw = this.draw || window.draw;
  if (typeof userDraw === 'function') {
    if (typeof userSetup === 'undefined') {
      this.scale(this._pixelDensity, this._pixelDensity);
    }
    var self = this;
    var callMethod = function (f) {
      f.call(self);
    };
    for (var idxRedraw = 0; idxRedraw < numberOfRedraws; idxRedraw++) {
      this._registeredMethods.pre.forEach(callMethod);
      userDraw();
      this._registeredMethods.post.forEach(callMethod);
    }
  }
};

p5.prototype.size = function() {
  var s = 'size() is not a valid p5 function, to set the size of the ';
  s += 'drawing canvas, please use createCanvas() instead';
  throw s;
};


module.exports = p5;

},{"./core":54}],66:[function(_dereq_,module,exports){
/**
 * @module Transform
 * @submodule Transform
 * @for p5
 * @requires core
 * @requires constants
 */


'use strict';

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');

/**
 * Multiplies the current matrix by the one specified through the parameters.
 * This is very slow because it will try to calculate the inverse of the
 * transform, so avoid it whenever possible.
 *
 * @method applyMatrix
 * @param  {Number} n00 numbers which define the 3x2 matrix to be multiplied
 * @param  {Number} n01 numbers which define the 3x2 matrix to be multiplied
 * @param  {Number} n02 numbers which define the 3x2 matrix to be multiplied
 * @param  {Number} n10 numbers which define the 3x2 matrix to be multiplied
 * @param  {Number} n11 numbers which define the 3x2 matrix to be multiplied
 * @param  {Number} n12 numbers which define the 3x2 matrix to be multiplied
 * @chainable
 * @example
 * <div>
 * <code>
 * // Example in the works.
 * </code>
 * </div>
 *
 * @alt
 * no image diplayed
 *
 */
p5.prototype.applyMatrix = function(n00, n01, n02, n10, n11, n12) {
  this._renderer.applyMatrix(n00, n01, n02, n10, n11, n12);
  return this;
};

p5.prototype.popMatrix = function() {
  throw new Error('popMatrix() not used, see pop()');
};

p5.prototype.printMatrix = function() {
  throw new Error('printMatrix() not implemented');
};

p5.prototype.pushMatrix = function() {
  throw new Error('pushMatrix() not used, see push()');
};

/**
 * Replaces the current matrix with the identity matrix.
 *
 * @method resetMatrix
 * @chainable
 * @example
 * <div>
 * <code>
 * // Example in the works.
 * </code>
 * </div>
 *
 * @alt
 * no image diplayed
 *
 */
p5.prototype.resetMatrix = function() {
  this._renderer.resetMatrix();
  return this;
};

/**
 * Rotates a shape the amount specified by the angle parameter. This
 * function accounts for angleMode, so angles can be entered in either
 * RADIANS or DEGREES.
 * <br><br>
 * Objects are always rotated around their relative position to the
 * origin and positive numbers rotate objects in a clockwise direction.
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * rotate(HALF_PI) and then rotate(HALF_PI) is the same as rotate(PI).
 * All tranformations are reset when draw() begins again.
 * <br><br>
 * Technically, rotate() multiplies the current transformation matrix
 * by a rotation matrix. This function can be further controlled by
 * the push() and pop().
 *
 * @method rotate
 * @param  {Number} angle the angle of rotation, specified in radians
 *                        or degrees, depending on current angleMode
 * @param  {p5.Vector|Array} [axis] (in 3d) the axis to rotate around
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(width/2, height/2);
 * rotate(PI/3.0);
 * rect(-26, -26, 52, 52);
 * </code>
 * </div>
 *
 * @alt
 * white 52x52 rect with black outline at center rotated counter 45 degrees
 *
 */
p5.prototype.rotate = function(angle, axis) {
  var args = new Array(arguments.length);
  var r;
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if (this._angleMode === constants.DEGREES) {
    r = this.radians(args[0]);
  } else if (this._angleMode === constants.RADIANS){
    r = args[0];
  }
  //in webgl mode
  if(args.length > 1){
    this._renderer.rotate(r, args[1]);
  }
  else {
    this._renderer.rotate(r);
  }
  return this;
};

/**
 * Rotates around X axis.
 * @method  rotateX
 * @param  {Number} rad angles in radians
 * @chainable
 */
p5.prototype.rotateX = function(rad) {
  if (this._renderer.isP3D) {
    this._renderer.rotateX(rad);
  } else {
    throw 'not supported in p2d. Please use webgl mode';
  }
  return this;
};

/**
 * Rotates around Y axis.
 * @method rotateY
 * @param  {Number} rad angles in radians
 * @chainable
 */
p5.prototype.rotateY = function(rad) {
  if (this._renderer.isP3D) {
    this._renderer.rotateY(rad);
  } else {
    throw 'not supported in p2d. Please use webgl mode';
  }
  return this;
};

/**
 * Rotates around Z axis. Webgl mode only.
 * @method rotateZ
 * @param  {Number} rad angles in radians
 * @chainable
 */
p5.prototype.rotateZ = function(rad) {
  if (this._renderer.isP3D) {
    this._renderer.rotateZ(rad);
  } else {
    throw 'not supported in p2d. Please use webgl mode';
  }
  return this;
};

/**
 * Increases or decreases the size of a shape by expanding and contracting
 * vertices. Objects always scale from their relative origin to the
 * coordinate system. Scale values are specified as decimal percentages.
 * For example, the function call scale(2.0) increases the dimension of a
 * shape by 200%.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function multiply the effect. For example, calling scale(2.0)
 * and then scale(1.5) is the same as scale(3.0). If scale() is called
 * within draw(), the transformation is reset when the loop begins again.
 * <br><br>
 * Using this function with the z parameter is only available in WEBGL mode.
 * This function can be further controlled with push() and pop().
 *
 * @method scale
 * @param  {Number|p5.Vector|Array} s
 *                      percent to scale the object, or percentage to
 *                      scale the object in the x-axis if multiple arguments
 *                      are given
 * @param  {Number} [y] percent to scale the object in the y-axis
 * @param  {Number} [z] percent to scale the object in the z-axis (webgl only)
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(width/2, height/2);
 * rotate(PI/3.0);
 * rect(-26, -26, 52, 52);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * rect(30, 20, 50, 50);
 * scale(0.5, 1.3);
 * rect(30, 20, 50, 50);
 * </code>
 * </div>
 *
 * @alt
 * white 52x52 rect with black outline at center rotated counter 45 degrees
 * 2 white rects with black outline- 1 50x50 at center. other 25x65 bottom left
 *
 */
p5.prototype.scale = function() {
  var x,y,z;
  var args = new Array(arguments.length);
  for(var i = 0; i < args.length; i++) {
    args[i] = arguments[i];
  }
  // Only check for Vector argument type if Vector is available
  if (typeof p5.Vector !== 'undefined' && args[0] instanceof p5.Vector) {
    x = args[0].x;
    y = args[0].y;
    z = args[0].z;
  }
  else if(args[0] instanceof Array){
    x = args[0][0];
    y = args[0][1];
    z = args[0][2] || 1;
  }
  else {
    if(args.length === 1){
      x = y = z = args[0];
    }
    else {
      x = args[0];
      y = args[1];
      z = args[2] || 1;
    }
  }

  if(this._renderer.isP3D){
    this._renderer.scale.call(this._renderer, x,y,z);
  }
  else {
    this._renderer.scale.call(this._renderer, x,y);
  }
  return this;
};

/**
 * Shears a shape around the x-axis the amount specified by the angle
 * parameter. Angles should be specified in the current angleMode.
 * Objects are always sheared around their relative position to the origin
 * and positive numbers shear objects in a clockwise direction.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * shearX(PI/2) and then shearX(PI/2) is the same as shearX(PI).
 * If shearX() is called within the draw(), the transformation is reset when
 * the loop begins again.
 * <br><br>
 * Technically, shearX() multiplies the current transformation matrix by a
 * rotation matrix. This function can be further controlled by the
 * push() and pop() functions.
 *
 * @method shearX
 * @param  {Number} angle angle of shear specified in radians or degrees,
 *                        depending on current angleMode
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(width/4, height/4);
 * shearX(PI/4.0);
 * rect(0, 0, 30, 30);
 * </code>
 * </div>
 *
 * @alt
  * white irregular quadrilateral with black outline at top middle.
 *
 */
p5.prototype.shearX = function(angle) {
  if (this._angleMode === constants.DEGREES) {
    angle = this.radians(angle);
  }
  this._renderer.shearX(angle);
  return this;
};

/**
 * Shears a shape around the y-axis the amount specified by the angle
 * parameter. Angles should be specified in the current angleMode. Objects
 * are always sheared around their relative position to the origin and
 * positive numbers shear objects in a clockwise direction.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * shearY(PI/2) and then shearY(PI/2) is the same as shearY(PI). If
 * shearY() is called within the draw(), the transformation is reset when
 * the loop begins again.
 * <br><br>
 * Technically, shearY() multiplies the current transformation matrix by a
 * rotation matrix. This function can be further controlled by the
 * push() and pop() functions.
 *
 * @method shearY
 * @param  {Number} angle angle of shear specified in radians or degrees,
 *                        depending on current angleMode
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(width/4, height/4);
 * shearY(PI/4.0);
 * rect(0, 0, 30, 30);
 * </code>
 * </div>
 *
 * @alt
 * white irregular quadrilateral with black outline at middle bottom.
 *
 */
p5.prototype.shearY = function(angle) {
  if (this._angleMode === constants.DEGREES) {
    angle = this.radians(angle);
  }
  this._renderer.shearY(angle);
  return this;
};

/**
 * Specifies an amount to displace objects within the display window.
 * The x parameter specifies left/right translation, the y parameter
 * specifies up/down translation.
 * <br><br>
 * Transformations are cumulative and apply to everything that happens after
 * and subsequent calls to the function accumulates the effect. For example,
 * calling translate(50, 0) and then translate(20, 0) is the same as
 * translate(70, 0). If translate() is called within draw(), the
 * transformation is reset when the loop begins again. This function can be
 * further controlled by using push() and pop().
 *
 * @method translate
 * @param  {Number} x left/right translation
 * @param  {Number} y up/down translation
 * @param  {Number} [z] forward/backward translation (webgl only)
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(30, 20);
 * rect(0, 0, 55, 55);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * rect(0, 0, 55, 55);  // Draw rect at original 0,0
 * translate(30, 20);
 * rect(0, 0, 55, 55);  // Draw rect at new 0,0
 * translate(14, 14);
 * rect(0, 0, 55, 55);  // Draw rect at new 0,0
 * </code>
 * </div>
 *
 * @alt
 * white 55x55 rect with black outline at center right.
 * 3 white 55x55 rects with black outlines at top-l, center-r and bottom-r.
 *
 */
p5.prototype.translate = function(x, y, z) {
  if (this._renderer.isP3D) {
    this._renderer.translate(x, y, z);
  } else {
    this._renderer.translate(x, y);
  }
  return this;
};

module.exports = p5;

},{"./constants":53,"./core":54}],67:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule Vertex
 * @for p5
 * @requires core
 * @requires constants
 */

'use strict';

var p5 = _dereq_('./core');
var constants = _dereq_('./constants');
var shapeKind = null;
var vertices = [];
var contourVertices = [];
var isBezier = false;
var isCurve = false;
var isQuadratic = false;
var isContour = false;
var isFirstContour = true;

/**
 * Use the beginContour() and endContour() functions to create negative
 * shapes within shapes such as the center of the letter 'O'. beginContour()
 * begins recording vertices for the shape and endContour() stops recording.
 * The vertices that define a negative shape must "wind" in the opposite
 * direction from the exterior shape. First draw vertices for the exterior
 * clockwise order, then for internal shapes, draw vertices
 * shape in counter-clockwise.
 * <br><br>
 * These functions can only be used within a beginShape()/endShape() pair and
 * transformations such as translate(), rotate(), and scale() do not work
 * within a beginContour()/endContour() pair. It is also not possible to use
 * other shapes, such as ellipse() or rect() within.
 *
 * @method beginContour
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(50, 50);
 * stroke(255, 0, 0);
 * beginShape();
 * // Exterior part of shape, clockwise winding
 * vertex(-40, -40);
 * vertex(40, -40);
 * vertex(40, 40);
 * vertex(-40, 40);
 * // Interior part of shape, counter-clockwise winding
 * beginContour();
 * vertex(-20, -20);
 * vertex(-20, 20);
 * vertex(20, 20);
 * vertex(20, -20);
 * endContour();
 * endShape(CLOSE);
 * </code>
 * </div>
 *
 * @alt
 * white rect and smaller grey rect with red outlines in center of canvas.
 *
 */
p5.prototype.beginContour = function() {
  contourVertices = [];
  isContour = true;
  return this;
};

/**
 * Using the beginShape() and endShape() functions allow creating more
 * complex forms. beginShape() begins recording vertices for a shape and
 * endShape() stops recording. The value of the kind parameter tells it which
 * types of shapes to create from the provided vertices. With no mode
 * specified, the shape can be any irregular polygon.
 * <br><br>
 * The parameters available for beginShape() are POINTS, LINES, TRIANGLES,
 * TRIANGLE_FAN, TRIANGLE_STRIP, QUADS, and QUAD_STRIP. After calling the
 * beginShape() function, a series of vertex() commands must follow. To stop
 * drawing the shape, call endShape(). Each shape will be outlined with the
 * current stroke color and filled with the fill color.
 * <br><br>
 * Transformations such as translate(), rotate(), and scale() do not work
 * within beginShape(). It is also not possible to use other shapes, such as
 * ellipse() or rect() within beginShape().
 *
 * @method beginShape
 * @param  {Constant} [kind] either POINTS, LINES, TRIANGLES, TRIANGLE_FAN
 *                                TRIANGLE_STRIP, QUADS, or QUAD_STRIP
 * @chainable
 * @example
 * <div>
 * <code>
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape(CLOSE);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * // currently not working
 * beginShape(POINTS);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(LINES);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape(CLOSE);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(TRIANGLES);
 * vertex(30, 75);
 * vertex(40, 20);
 * vertex(50, 75);
 * vertex(60, 20);
 * vertex(70, 75);
 * vertex(80, 20);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(TRIANGLE_STRIP);
 * vertex(30, 75);
 * vertex(40, 20);
 * vertex(50, 75);
 * vertex(60, 20);
 * vertex(70, 75);
 * vertex(80, 20);
 * vertex(90, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(TRIANGLE_FAN);
 * vertex(57.5, 50);
 * vertex(57.5, 15);
 * vertex(92, 50);
 * vertex(57.5, 85);
 * vertex(22, 50);
 * vertex(57.5, 15);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(QUADS);
 * vertex(30, 20);
 * vertex(30, 75);
 * vertex(50, 75);
 * vertex(50, 20);
 * vertex(65, 20);
 * vertex(65, 75);
 * vertex(85, 75);
 * vertex(85, 20);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape(QUAD_STRIP);
 * vertex(30, 20);
 * vertex(30, 75);
 * vertex(50, 20);
 * vertex(50, 75);
 * vertex(65, 20);
 * vertex(65, 75);
 * vertex(85, 20);
 * vertex(85, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape();
 * vertex(20, 20);
 * vertex(40, 20);
 * vertex(40, 40);
 * vertex(60, 40);
 * vertex(60, 60);
 * vertex(20, 60);
 * endShape(CLOSE);
 * </code>
 * </div>
  * @alt
 * white square-shape with black outline in middle-right of canvas.
 * 4 black points in a square shape in middle-right of canvas.
 * 2 horizontal black lines. In the top-right and bottom-right of canvas.
 * 3 line shape with horizontal on top, vertical in middle and horizontal bottom.
 * square line shape in middle-right of canvas.
 * 2 white triangle shapes mid-right canvas. left one pointing up and right down.
 * 5 horizontal interlocking and alternating white triangles in mid-right canvas.
 * 4 interlocking white triangles in 45 degree rotated square-shape.
 * 2 white rectangle shapes in mid-right canvas. Both 20x55.
 * 3 side-by-side white rectangles center rect is smaller in mid-right canvas.
 * Thick white l-shape with black outline mid-top-left of canvas.
 *
 */
p5.prototype.beginShape = function(kind) {
  if (kind === constants.POINTS ||
    kind === constants.LINES ||
    kind === constants.TRIANGLES ||
    kind === constants.TRIANGLE_FAN ||
    kind === constants.TRIANGLE_STRIP ||
    kind === constants.QUADS ||
    kind === constants.QUAD_STRIP) {
    shapeKind = kind;
  } else {
    shapeKind = null;
  }
  if(this._renderer.isP3D){
    this._renderer.beginShape(kind);
  } else {
    vertices = [];
    contourVertices = [];
  }
  return this;
};

/**
 * Specifies vertex coordinates for Bezier curves. Each call to
 * bezierVertex() defines the position of two control points and
 * one anchor point of a Bezier curve, adding a new segment to a
 * line or shape.
 * <br><br>
 * The first time bezierVertex() is used within a
 * beginShape() call, it must be prefaced with a call to vertex()
 * to set the first anchor point. This function must be used between
 * beginShape() and endShape() and only when there is no MODE
 * parameter specified to beginShape().
 *
 * @method bezierVertex
 * @param  {Number} x2 x-coordinate for the first control point
 * @param  {Number} y2 y-coordinate for the first control point
 * @param  {Number} x3 x-coordinate for the second control point
 * @param  {Number} y3 y-coordinate for the second control point
 * @param  {Number} x4 x-coordinate for the anchor point
 * @param  {Number} y4 y-coordinate for the anchor point
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 * beginShape();
 * vertex(30, 20);
 * bezierVertex(80, 0, 80, 75, 30, 75);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * beginShape();
 * vertex(30, 20);
 * bezierVertex(80, 0, 80, 75, 30, 75);
 * bezierVertex(50, 80, 60, 25, 30, 20);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * crescent-shaped line in middle of canvas. Points facing left.
 * white crescent shape in middle of canvas. Points facing left.
 *
 */
p5.prototype.bezierVertex = function(x2, y2, x3, y3, x4, y4) {
  if (vertices.length === 0) {
    throw 'vertex() must be used once before calling bezierVertex()';
  } else {
    isBezier = true;
    var vert = [];
    for (var i = 0; i < arguments.length; i++) {
      vert[i] = arguments[i];
    }
    vert.isVert = false;
    if (isContour) {
      contourVertices.push(vert);
    } else {
      vertices.push(vert);
    }
  }
  return this;
};

/**
 * Specifies vertex coordinates for curves. This function may only
 * be used between beginShape() and endShape() and only when there
 * is no MODE parameter specified to beginShape().
 * <br><br>
 * The first and last points in a series of curveVertex() lines will be used to
 * guide the beginning and end of a the curve. A minimum of four
 * points is required to draw a tiny curve between the second and
 * third points. Adding a fifth point with curveVertex() will draw
 * the curve between the second, third, and fourth points. The
 * curveVertex() function is an implementation of Catmull-Rom
 * splines.
 *
 * @method curveVertex
 * @param {Number} x x-coordinate of the vertex
 * @param {Number} y y-coordinate of the vertex
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 * beginShape();
 * curveVertex(84,  91);
 * curveVertex(84,  91);
 * curveVertex(68,  19);
 * curveVertex(21,  17);
 * curveVertex(32, 100);
 * curveVertex(32, 100);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * Upside-down u-shape line, mid canvas. left point extends beyond canvas view.
 *
 */
p5.prototype.curveVertex = function(x,y) {
  isCurve = true;
  this.vertex(x, y);
  return this;
};

/**
 * Use the beginContour() and endContour() functions to create negative
 * shapes within shapes such as the center of the letter 'O'. beginContour()
 * begins recording vertices for the shape and endContour() stops recording.
 * The vertices that define a negative shape must "wind" in the opposite
 * direction from the exterior shape. First draw vertices for the exterior
 * clockwise order, then for internal shapes, draw vertices
 * shape in counter-clockwise.
 * <br><br>
 * These functions can only be used within a beginShape()/endShape() pair and
 * transformations such as translate(), rotate(), and scale() do not work
 * within a beginContour()/endContour() pair. It is also not possible to use
 * other shapes, such as ellipse() or rect() within.
 *
 * @method endContour
 * @chainable
 * @example
 * <div>
 * <code>
 * translate(50, 50);
 * stroke(255, 0, 0);
 * beginShape();
 * // Exterior part of shape, clockwise winding
 * vertex(-40, -40);
 * vertex(40, -40);
 * vertex(40, 40);
 * vertex(-40, 40);
 * // Interior part of shape, counter-clockwise winding
 * beginContour();
 * vertex(-20, -20);
 * vertex(-20, 20);
 * vertex(20, 20);
 * vertex(20, -20);
 * endContour();
 * endShape(CLOSE);
 * </code>
 * </div>
 *
 * @alt
 * white rect and smaller grey rect with red outlines in center of canvas.
 *
 */
p5.prototype.endContour = function() {
  var vert = contourVertices[0].slice(); // copy all data
  vert.isVert = contourVertices[0].isVert;
  vert.moveTo = false;
  contourVertices.push(vert);

  // prevent stray lines with multiple contours
  if (isFirstContour) {
    vertices.push(vertices[0]);
    isFirstContour = false;
  }

  for (var i = 0; i < contourVertices.length; i++) {
    vertices.push(contourVertices[i]);
  }
  return this;
};

/**
 * The endShape() function is the companion to beginShape() and may only be
 * called after beginShape(). When endshape() is called, all of image data
 * defined since the previous call to beginShape() is written into the image
 * buffer. The constant CLOSE as the value for the MODE parameter to close
 * the shape (to connect the beginning and the end).
 *
 * @method endShape
 * @param  {Constant} [mode] use CLOSE to close the shape
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 *
 * beginShape();
 * vertex(20, 20);
 * vertex(45, 20);
 * vertex(45, 80);
 * endShape(CLOSE);
 *
 * beginShape();
 * vertex(50, 20);
 * vertex(75, 20);
 * vertex(75, 80);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * Triangle line shape with smallest interior angle on bottom and upside-down L.
 *
 */
p5.prototype.endShape = function(mode) {
  if(this._renderer.isP3D){
    this._renderer.endShape(mode, isCurve, isBezier,
      isQuadratic, isContour, shapeKind);
  }else{
    if (vertices.length === 0) { return this; }
    if (!this._renderer._doStroke && !this._renderer._doFill) { return this; }

    var closeShape = mode === constants.CLOSE;

    // if the shape is closed, the first element is also the last element
    if (closeShape && !isContour) {
      vertices.push(vertices[0]);
    }

    this._renderer.endShape(mode, vertices, isCurve, isBezier,
      isQuadratic, isContour, shapeKind);

    // Reset some settings
    isCurve = false;
    isBezier = false;
    isQuadratic = false;
    isContour = false;
    isFirstContour = true;

    // If the shape is closed, the first element was added as last element.
    // We must remove it again to prevent the list of vertices from growing
    // over successive calls to endShape(CLOSE)
    if (closeShape) {
      vertices.pop();
    }
  }
  return this;
};

/**
 * Specifies vertex coordinates for quadratic Bezier curves. Each call to
 * quadraticVertex() defines the position of one control points and one
 * anchor point of a Bezier curve, adding a new segment to a line or shape.
 * The first time quadraticVertex() is used within a beginShape() call, it
 * must be prefaced with a call to vertex() to set the first anchor point.
 * This function must be used between beginShape() and endShape() and only
 * when there is no MODE parameter specified to beginShape().
 *
 * @method quadraticVertex
 * @param  {Number} cx x-coordinate for the control point
 * @param  {Number} cy y-coordinate for the control point
 * @param  {Number} x3 x-coordinate for the anchor point
 * @param  {Number} y3 y-coordinate for the anchor point
 * @chainable
 * @example
 * <div>
 * <code>
 * noFill();
 * strokeWeight(4);
 * beginShape();
 * vertex(20, 20);
 * quadraticVertex(80, 20, 50, 50);
 * endShape();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * noFill();
 * strokeWeight(4);
 * beginShape();
 * vertex(20, 20);
 * quadraticVertex(80, 20, 50, 50);
 * quadraticVertex(20, 80, 80, 80);
 * vertex(80, 60);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * arched-shaped black line with 4 pixel thick stroke weight.
 * backwards s-shaped black line with 4 pixel thick stroke weight.
 *
 */
p5.prototype.quadraticVertex = function(cx, cy, x3, y3) {
  //if we're drawing a contour, put the points into an
  // array for inside drawing
  if(this._contourInited) {
    var pt = {};
    pt.x = cx;
    pt.y = cy;
    pt.x3 = x3;
    pt.y3 = y3;
    pt.type = constants.QUADRATIC;
    this._contourVertices.push(pt);

    return this;
  }
  if (vertices.length > 0) {
    isQuadratic = true;
    var vert = [];
    for (var i = 0; i < arguments.length; i++) {
      vert[i] = arguments[i];
    }
    vert.isVert = false;
    if (isContour) {
      contourVertices.push(vert);
    } else {
      vertices.push(vert);
    }
  } else {
    throw 'vertex() must be used once before calling quadraticVertex()';
  }
  return this;
};

/**
 * All shapes are constructed by connecting a series of vertices. vertex()
 * is used to specify the vertex coordinates for points, lines, triangles,
 * quads, and polygons. It is used exclusively within the beginShape() and
 * endShape() functions.
 *
 * @method vertex
 * @param  {Number} x x-coordinate of the vertex
 * @param  {Number} y y-coordinate of the vertex
 * @param  {Number|Boolean} [z] z-coordinate of the vertex
 * @chainable
 * @example
 * <div>
 * <code>
 * beginShape(POINTS);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 * </code>
 * </div>
 *
 * @alt
 * 4 black points in a square shape in middle-right of canvas.
 *
 */
p5.prototype.vertex = function(x, y, moveTo) {
  if(this._renderer.isP3D){
    this._renderer.vertex(x, y, moveTo);
  }else{
    var vert = [];
    vert.isVert = true;
    vert[0] = x;
    vert[1] = y;
    vert[2] = 0;
    vert[3] = 0;
    vert[4] = 0;
    vert[5] = this._renderer._getFill();
    vert[6] = this._renderer._getStroke();

    if (moveTo) {
      vert.moveTo = moveTo;
    }
    if (isContour) {
      if (contourVertices.length === 0) {
        vert.moveTo = true;
      }
      contourVertices.push(vert);
    } else {
      vertices.push(vert);
    }
  }
  return this;
};

module.exports = p5;

},{"./constants":53,"./core":54}],73:[function(_dereq_,module,exports){
/*global ImageData:false */

/**
 * This module defines the filters for use with image buffers.
 *
 * This module is basically a collection of functions stored in an object
 * as opposed to modules. The functions are destructive, modifying
 * the passed in canvas rather than creating a copy.
 *
 * Generally speaking users of this module will use the Filters.apply method
 * on a canvas to create an effect.
 *
 * A number of functions are borrowed/adapted from
 * http://www.html5rocks.com/en/tutorials/canvas/imagefilters/
 * or the java processing implementation.
 */

'use strict';

var Filters = {};


/*
 * Helper functions
 */


/**
 * Returns the pixel buffer for a canvas
 *
 * @private
 *
 * @param  {Canvas|ImageData} canvas the canvas to get pixels from
 * @return {Uint8ClampedArray}       a one-dimensional array containing
 *                                   the data in thc RGBA order, with integer
 *                                   values between 0 and 255
 */
Filters._toPixels = function (canvas) {
  if (canvas instanceof ImageData) {
    return canvas.data;
  } else {
    return canvas.getContext('2d').getImageData(
      0,
      0,
      canvas.width,
      canvas.height
    ).data;
  }
};

/**
 * Returns a 32 bit number containing ARGB data at ith pixel in the
 * 1D array containing pixels data.
 *
 * @private
 *
 * @param  {Uint8ClampedArray} data array returned by _toPixels()
 * @param  {Integer}           i    index of a 1D Image Array
 * @return {Integer}                32 bit integer value representing
 *                                  ARGB value.
 */
Filters._getARGB = function (data, i) {
  var offset = i * 4;
  return (data[offset+3] << 24) & 0xff000000 |
    (data[offset] << 16) & 0x00ff0000 |
    (data[offset+1] << 8) & 0x0000ff00 |
    data[offset+2] & 0x000000ff;
};

/**
 * Modifies pixels RGBA values to values contained in the data object.
 *
 * @private
 *
 * @param {Uint8ClampedArray} pixels array returned by _toPixels()
 * @param {Int32Array}        data   source 1D array where each value
 *                                   represents ARGB values
 */
Filters._setPixels = function (pixels, data) {
  var offset = 0;
  for( var i = 0, al = pixels.length; i < al; i++) {
    offset = i*4;
    pixels[offset + 0] = (data[i] & 0x00ff0000)>>>16;
    pixels[offset + 1] = (data[i] & 0x0000ff00)>>>8;
    pixels[offset + 2] = (data[i] & 0x000000ff);
    pixels[offset + 3] = (data[i] & 0xff000000)>>>24;
  }
};

/**
 * Returns the ImageData object for a canvas
 * https://developer.mozilla.org/en-US/docs/Web/API/ImageData
 *
 * @private
 *
 * @param  {Canvas|ImageData} canvas canvas to get image data from
 * @return {ImageData}               Holder of pixel data (and width and
 *                                   height) for a canvas
 */
Filters._toImageData = function (canvas) {
  if (canvas instanceof ImageData) {
    return canvas;
  } else {
    return canvas.getContext('2d').getImageData(
      0,
      0,
      canvas.width,
      canvas.height
    );
  }
};

/**
 * Returns a blank ImageData object.
 *
 * @private
 *
 * @param  {Integer} width
 * @param  {Integer} height
 * @return {ImageData}
 */
Filters._createImageData = function (width, height) {
  Filters._tmpCanvas = document.createElement('canvas');
  Filters._tmpCtx = Filters._tmpCanvas.getContext('2d');
  return this._tmpCtx.createImageData(width, height);
};


/**
 * Applys a filter function to a canvas.
 *
 * The difference between this and the actual filter functions defined below
 * is that the filter functions generally modify the pixel buffer but do
 * not actually put that data back to the canvas (where it would actually
 * update what is visible). By contrast this method does make the changes
 * actually visible in the canvas.
 *
 * The apply method is the method that callers of this module would generally
 * use. It has been separated from the actual filters to support an advanced
 * use case of creating a filter chain that executes without actually updating
 * the canvas in between everystep.
 *
 * @param  {HTMLCanvasElement} canvas [description]
 * @param  {function(ImageData,Object)} func   [description]
 * @param  {Object} filterParam  [description]
 */
Filters.apply = function (canvas, func, filterParam) {
  var ctx = canvas.getContext('2d');
  var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

  //Filters can either return a new ImageData object, or just modify
  //the one they received.
  var newImageData = func(imageData, filterParam);
  if (newImageData instanceof ImageData) {
    ctx.putImageData(newImageData, 0, 0, 0, 0, canvas.width, canvas.height);
  } else {
    ctx.putImageData(imageData, 0, 0, 0, 0, canvas.width, canvas.height);
  }
};


/*
 * Filters
 */


/**
 * Converts the image to black and white pixels depending if they are above or
 * below the threshold defined by the level parameter. The parameter must be
 * between 0.0 (black) and 1.0 (white). If no level is specified, 0.5 is used.
 *
 * Borrowed from http://www.html5rocks.com/en/tutorials/canvas/imagefilters/
 *
 * @param  {Canvas} canvas
 * @param  {Float} level
 */
Filters.threshold = function (canvas, level) {
  var pixels = Filters._toPixels(canvas);

  if (level === undefined) {
    level = 0.5;
  }
  var thresh = Math.floor(level * 255);

  for (var i = 0; i < pixels.length; i += 4) {
    var r = pixels[i];
    var g = pixels[i + 1];
    var b = pixels[i + 2];
    var gray = (0.2126 * r + 0.7152 * g + 0.0722 * b);
    var val;
    if (gray >= thresh) {
      val = 255;
    } else {
      val = 0;
    }
    pixels[i] = pixels[i + 1] = pixels[i + 2] = val;
  }

};


/**
 * Converts any colors in the image to grayscale equivalents.
 * No parameter is used.
 *
 * Borrowed from http://www.html5rocks.com/en/tutorials/canvas/imagefilters/
 *
 * @param {Canvas} canvas
 */
Filters.gray = function (canvas) {
  var pixels = Filters._toPixels(canvas);

  for (var i = 0; i < pixels.length; i += 4) {
    var r = pixels[i];
    var g = pixels[i + 1];
    var b = pixels[i + 2];

    // CIE luminance for RGB
    var gray = (0.2126 * r + 0.7152 * g + 0.0722 * b);
    pixels[i] = pixels[i + 1] = pixels[i + 2] = gray;
  }
};

/**
 * Sets the alpha channel to entirely opaque. No parameter is used.
 *
 * @param {Canvas} canvas
 */
Filters.opaque = function (canvas) {
  var pixels = Filters._toPixels(canvas);

  for (var i = 0; i < pixels.length; i += 4) {
    pixels[i + 3] = 255;
  }

  return pixels;
};

/**
 * Sets each pixel to its inverse value. No parameter is used.
 * @param  {Canvas} canvas
 */
Filters.invert = function (canvas) {
  var pixels = Filters._toPixels(canvas);

  for (var i = 0; i < pixels.length; i += 4) {
    pixels[i] = 255 - pixels[i];
    pixels[i + 1] = 255 - pixels[i + 1];
    pixels[i + 2] = 255 - pixels[i + 2];
  }

};


/**
 * Limits each channel of the image to the number of colors specified as
 * the parameter. The parameter can be set to values between 2 and 255, but
 * results are most noticeable in the lower ranges.
 *
 * Adapted from java based processing implementation
 *
 * @param  {Canvas} canvas
 * @param  {Integer} level
 */
Filters.posterize = function (canvas, level) {
  var pixels = Filters._toPixels(canvas);

  if ((level < 2) || (level > 255)) {
    throw new Error(
      'Level must be greater than 2 and less than 255 for posterize'
    );
  }

  var levels1 = level - 1;
  for (var i = 0; i < pixels.length; i+=4) {
    var rlevel = pixels[i];
    var glevel = pixels[i + 1];
    var blevel = pixels[i + 2];

    pixels[i] = (((rlevel * level) >> 8) * 255) / levels1;
    pixels[i + 1] = (((glevel * level) >> 8) * 255) / levels1;
    pixels[i + 2] = (((blevel * level) >> 8) * 255) / levels1;
  }
};

/**
 * reduces the bright areas in an image
 * @param  {Canvas} canvas
 *
 */
Filters.dilate = function (canvas) {
  var pixels = Filters._toPixels(canvas);
  var currIdx = 0;
  var maxIdx = pixels.length ? pixels.length/4 : 0;
  var out = new Int32Array(maxIdx);
  var currRowIdx, maxRowIdx, colOrig, colOut, currLum;
  var idxRight, idxLeft, idxUp, idxDown,
      colRight, colLeft, colUp, colDown,
      lumRight, lumLeft, lumUp, lumDown;

  while(currIdx < maxIdx) {
    currRowIdx = currIdx;
    maxRowIdx = currIdx + canvas.width;
    while (currIdx < maxRowIdx) {
      colOrig = colOut = Filters._getARGB(pixels, currIdx);
      idxLeft = currIdx - 1;
      idxRight = currIdx + 1;
      idxUp = currIdx - canvas.width;
      idxDown = currIdx + canvas.width;

      if (idxLeft < currRowIdx) {
        idxLeft = currIdx;
      }
      if (idxRight >= maxRowIdx) {
        idxRight = currIdx;
      }
      if (idxUp < 0){
        idxUp = 0;
      }
      if (idxDown >= maxIdx) {
        idxDown = currIdx;
      }
      colUp = Filters._getARGB(pixels, idxUp);
      colLeft = Filters._getARGB(pixels, idxLeft);
      colDown = Filters._getARGB(pixels, idxDown);
      colRight = Filters._getARGB(pixels, idxRight);

      //compute luminance
      currLum = 77*(colOrig>>16&0xff) +
        151*(colOrig>>8&0xff) +
        28*(colOrig&0xff);
      lumLeft = 77*(colLeft>>16&0xff) +
        151*(colLeft>>8&0xff) +
        28*(colLeft&0xff);
      lumRight = 77*(colRight>>16&0xff) +
        151*(colRight>>8&0xff) +
        28*(colRight&0xff);
      lumUp = 77*(colUp>>16&0xff) +
        151*(colUp>>8&0xff) +
        28*(colUp&0xff);
      lumDown = 77*(colDown>>16&0xff) +
        151*(colDown>>8&0xff) +
        28*(colDown&0xff);

      if (lumLeft > currLum) {
        colOut = colLeft;
        currLum = lumLeft;
      }
      if (lumRight > currLum) {
        colOut = colRight;
        currLum = lumRight;
      }
      if (lumUp > currLum) {
        colOut = colUp;
        currLum = lumUp;
      }
      if (lumDown > currLum) {
        colOut = colDown;
        currLum = lumDown;
      }
      out[currIdx++]=colOut;
    }
  }
  Filters._setPixels(pixels, out);
};

/**
 * increases the bright areas in an image
 * @param  {Canvas} canvas
 *
 */
Filters.erode = function(canvas) {
  var pixels = Filters._toPixels(canvas);
  var currIdx = 0;
  var maxIdx = pixels.length ? pixels.length/4 : 0;
  var out = new Int32Array(maxIdx);
  var currRowIdx, maxRowIdx, colOrig, colOut, currLum;
  var idxRight, idxLeft, idxUp, idxDown,
      colRight, colLeft, colUp, colDown,
      lumRight, lumLeft, lumUp, lumDown;

  while(currIdx < maxIdx) {
    currRowIdx = currIdx;
    maxRowIdx = currIdx + canvas.width;
    while (currIdx < maxRowIdx) {
      colOrig = colOut = Filters._getARGB(pixels, currIdx);
      idxLeft = currIdx - 1;
      idxRight = currIdx + 1;
      idxUp = currIdx - canvas.width;
      idxDown = currIdx + canvas.width;

      if (idxLeft < currRowIdx) {
        idxLeft = currIdx;
      }
      if (idxRight >= maxRowIdx) {
        idxRight = currIdx;
      }
      if (idxUp < 0) {
        idxUp = 0;
      }
      if (idxDown >= maxIdx) {
        idxDown = currIdx;
      }
      colUp = Filters._getARGB(pixels, idxUp);
      colLeft = Filters._getARGB(pixels, idxLeft);
      colDown = Filters._getARGB(pixels, idxDown);
      colRight = Filters._getARGB(pixels, idxRight);

      //compute luminance
      currLum = 77*(colOrig>>16&0xff) +
        151*(colOrig>>8&0xff) +
        28*(colOrig&0xff);
      lumLeft = 77*(colLeft>>16&0xff) +
        151*(colLeft>>8&0xff) +
        28*(colLeft&0xff);
      lumRight = 77*(colRight>>16&0xff) +
        151*(colRight>>8&0xff) +
        28*(colRight&0xff);
      lumUp = 77*(colUp>>16&0xff) +
        151*(colUp>>8&0xff) +
        28*(colUp&0xff);
      lumDown = 77*(colDown>>16&0xff) +
        151*(colDown>>8&0xff) +
        28*(colDown&0xff);

      if (lumLeft < currLum) {
        colOut = colLeft;
        currLum = lumLeft;
      }
      if (lumRight < currLum) {
        colOut = colRight;
        currLum = lumRight;
      }
      if (lumUp < currLum) {
        colOut = colUp;
        currLum = lumUp;
      }
      if (lumDown < currLum) {
        colOut = colDown;
        currLum = lumDown;
      }

      out[currIdx++]=colOut;
    }
  }
  Filters._setPixels(pixels, out);
};

// BLUR

// internal kernel stuff for the gaussian blur filter
var blurRadius;
var blurKernelSize;
var blurKernel;
var blurMult;

/*
 * Port of https://github.com/processing/processing/blob/
 * master/core/src/processing/core/PImage.java#L1250
 *
 * Optimized code for building the blur kernel.
 * further optimized blur code (approx. 15% for radius=20)
 * bigger speed gains for larger radii (~30%)
 * added support for various image types (ALPHA, RGB, ARGB)
 * [toxi 050728]
 */
function buildBlurKernel(r) {
  var radius = (r * 3.5)|0;
  radius = (radius < 1) ? 1 : ((radius < 248) ? radius : 248);

  if (blurRadius !== radius) {
    blurRadius = radius;
    blurKernelSize = 1 + blurRadius<<1;
    blurKernel = new Int32Array(blurKernelSize);
    blurMult = new Array(blurKernelSize);
    for(var l = 0; l < blurKernelSize; l++){
      blurMult[l] = new Int32Array(256);
    }

    var bk,bki;
    var bm,bmi;

    for (var i = 1, radiusi = radius - 1; i < radius; i++) {
      blurKernel[radius+i] = blurKernel[radiusi] = bki = radiusi * radiusi;
      bm = blurMult[radius+i];
      bmi = blurMult[radiusi--];
      for (var j = 0; j < 256; j++){
        bm[j] = bmi[j] = bki * j;
      }
    }
    bk = blurKernel[radius] = radius * radius;
    bm = blurMult[radius];

    for (var k = 0; k < 256; k++){
      bm[k] = bk * k;
    }
  }

}

// Port of https://github.com/processing/processing/blob/
// master/core/src/processing/core/PImage.java#L1433
function blurARGB(canvas, radius) {
  var pixels = Filters._toPixels(canvas);
  var width = canvas.width;
  var height = canvas.height;
  var numPackedPixels = width * height;
  var argb = new Int32Array(numPackedPixels);
  for (var j = 0; j < numPackedPixels; j++) {
    argb[j] = Filters._getARGB(pixels, j);
  }
  var sum, cr, cg, cb, ca;
  var read, ri, ym, ymi, bk0;
  var a2 = new Int32Array(numPackedPixels);
  var r2 = new Int32Array(numPackedPixels);
  var g2 = new Int32Array(numPackedPixels);
  var b2 = new Int32Array(numPackedPixels);
  var yi = 0;
  buildBlurKernel(radius);
  var x, y, i;
  var bm;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      cb = cg = cr = ca = sum = 0;
      read = x - blurRadius;
      if (read < 0) {
        bk0 = -read;
        read = 0;
      } else {
        if (read >= width) {
          break;
        }
        bk0 = 0;
      }
      for (i = bk0; i < blurKernelSize; i++) {
        if (read >= width) {
          break;
        }
        var c = argb[read + yi];
        bm = blurMult[i];
        ca += bm[(c & -16777216) >>> 24];
        cr += bm[(c & 16711680) >> 16];
        cg += bm[(c & 65280) >> 8];
        cb += bm[c & 255];
        sum += blurKernel[i];
        read++;
      }
      ri = yi + x;
      a2[ri] = ca / sum;
      r2[ri] = cr / sum;
      g2[ri] = cg / sum;
      b2[ri] = cb / sum;
    }
    yi += width;
  }
  yi = 0;
  ym = -blurRadius;
  ymi = ym * width;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      cb = cg = cr = ca = sum = 0;
      if (ym < 0) {
        bk0 = ri = -ym;
        read = x;
      } else {
        if (ym >= height) {
          break;
        }
        bk0 = 0;
        ri = ym;
        read = x + ymi;
      }
      for (i = bk0; i < blurKernelSize; i++) {
        if (ri >= height) {
          break;
        }
        bm = blurMult[i];
        ca += bm[a2[read]];
        cr += bm[r2[read]];
        cg += bm[g2[read]];
        cb += bm[b2[read]];
        sum += blurKernel[i];
        ri++;
        read += width;
      }
      argb[x + yi] = (ca/sum)<<24 | (cr/sum)<<16 | (cg/sum)<<8 | (cb/sum);
    }
    yi += width;
    ymi += width;
    ym++;
  }
  Filters._setPixels(pixels, argb);
}

Filters.blur = function(canvas, radius){
  blurARGB(canvas, radius);
};


module.exports = Filters;

},{}],74:[function(_dereq_,module,exports){
/**
 * @module Image
 * @submodule Image
 * @for p5
 * @requires core
 */

/**
 * This module defines the p5 methods for the p5.Image class
 * for drawing images to the main display canvas.
 */
'use strict';


var p5 = _dereq_('../core/core');

/* global frames:true */// This is not global, but JSHint is not aware that
// this module is implicitly enclosed with Browserify: this overrides the
// redefined-global error and permits using the name "frames" for the array
// of saved animation frames.
var frames = [];


/**
 * Creates a new p5.Image (the datatype for storing images). This provides a
 * fresh buffer of pixels to play with. Set the size of the buffer with the
 * width and height parameters.
 * <br><br>
 * .pixels gives access to an array containing the values for all the pixels
 * in the display window.
 * These values are numbers. This array is the size (including an appropriate
 * factor for the pixelDensity) of the display window x4,
 * representing the R, G, B, A values in order for each pixel, moving from
 * left to right across each row, then down each column. See .pixels for
 * more info. It may also be simpler to use set() or get().
 * <br><br>
 * Before accessing the pixels of an image, the data must loaded with the
 * loadPixels() function. After the array data has been modified, the
 * updatePixels() function must be run to update the changes.
 *
 * @method createImage
 * @param  {Integer} width  width in pixels
 * @param  {Integer} height height in pixels
 * @return {p5.Image}       the p5.Image object
 * @example
 * <div>
 * <code>
 * img = createImage(66, 66);
 * img.loadPixels();
 * for (i = 0; i < img.width; i++) {
 *   for (j = 0; j < img.height; j++) {
 *     img.set(i, j, color(0, 90, 102));
 *   }
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * img = createImage(66, 66);
 * img.loadPixels();
 * for (i = 0; i < img.width; i++) {
 *   for (j = 0; j < img.height; j++) {
 *     img.set(i, j, color(0, 90, 102, i % img.width * 2));
 *   }
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 * image(img, 34, 34);
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var pink = color(255, 102, 204);
 * img = createImage(66, 66);
 * img.loadPixels();
 * var d = pixelDensity();
 * var halfImage = 4 * (width * d) * (height/2 * d);
 * for (var i = 0; i < halfImage; i+=4) {
 *   img.pixels[i] = red(pink);
 *   img.pixels[i+1] = green(pink);
 *   img.pixels[i+2] = blue(pink);
 *   img.pixels[i+3] = alpha(pink);
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 * </code>
 * </div>
 *
 * @alt
 * 66x66 dark turquoise rect in center of canvas.
 * 2 gradated dark turquoise rects fade left. 1 center 1 bottom right of canvas
 * no image displayed
 *
 */
p5.prototype.createImage = function(width, height) {
  return new p5.Image(width, height);
};

/**
 *  Save the current canvas as an image. In Safari, this will open the
 *  image in the window and the user must provide their own
 *  filename on save-as. Other browsers will either save the
 *  file immediately, or prompt the user with a dialogue window.
 *
 *  @method saveCanvas
 *  @param  {p5.Element|HTMLCanvasElement} selectedCanvas   a variable
 *                                  representing a specific html5 canvas (optional)
 *  @param  {String} [filename]
 *  @param  {String} [extension]      'jpg' or 'png'
*/
/**
 *  @method saveCanvas
 *  @param  {String} [filename]
 *  @param  {String} [extension]
 *
 *  @example
 *  <div class='norender'><code>
 *  function setup() {
 *    var c = createCanvas(100, 100);
 *    background(255, 0, 0);
 *    saveCanvas(c, 'myCanvas', 'jpg');
 *  }
 *  </code></div>
 *  <div class='norender'><code>
 *  // note that this example has the same result as above
 *  // if no canvas is specified, defaults to main canvas
 *  function setup() {
 *    createCanvas(100, 100);
 *    background(255, 0, 0);
 *    saveCanvas('myCanvas', 'jpg');
 *  }
 *  </code></div>
 *  <div class='norender'><code>
 *  // all of the following are valid
 *  saveCanvas(c, 'myCanvas', 'jpg');
 *  saveCanvas(c, 'myCanvas');
 *  saveCanvas(c);
 *  saveCanvas('myCanvas', 'png');
 *  saveCanvas('myCanvas');
 *  saveCanvas();
 *  </code></div>
 *
 * @alt
 * no image displayed
 * no image displayed
 * no image displayed
 *
 */
p5.prototype.saveCanvas = function() {

  var cnv, filename, extension;
  if (arguments.length === 3) {
    cnv = arguments[0];
    filename = arguments[1];
    extension = arguments[2];
  } else if (arguments.length === 2) {
    if (typeof arguments[0] === 'object') {
      cnv = arguments[0];
      filename = arguments[1];
    } else {
      filename = arguments[0];
      extension = arguments[1];
    }
  } else if (arguments.length === 1) {
    if (typeof arguments[0] === 'object') {
      cnv = arguments[0];
    } else {
      filename = arguments[0];
    }
  }

  if (cnv instanceof p5.Element) {
    cnv = cnv.elt;
  }
  if (!(cnv instanceof HTMLCanvasElement)) {
    cnv = null;
  }

  if (!extension) {
    extension = p5.prototype._checkFileExtension(filename, extension)[1];
    if (extension === '') {
      extension = 'png';
    }
  }

  if (!cnv) {
    if (this._curElement && this._curElement.elt) {
      cnv = this._curElement.elt;
    }
  }

  if ( p5.prototype._isSafari() ) {
    var aText = 'Hello, Safari user!\n';
    aText += 'Now capturing a screenshot...\n';
    aText += 'To save this image,\n';
    aText += 'go to File --> Save As.\n';
    alert(aText);
    window.location.href = cnv.toDataURL();
  } else {
    var mimeType;
    if (typeof(extension) === 'undefined') {
      extension = 'png';
      mimeType = 'image/png';
    }
    else {
      switch(extension){
        case 'png':
          mimeType = 'image/png';
          break;
        case 'jpeg':
          mimeType = 'image/jpeg';
          break;
        case 'jpg':
          mimeType = 'image/jpeg';
          break;
        default:
          mimeType = 'image/png';
          break;
      }
    }
    var downloadMime = 'image/octet-stream';
    var imageData = cnv.toDataURL(mimeType);
    imageData = imageData.replace(mimeType, downloadMime);

    p5.prototype.downloadFile(imageData, filename, extension);
  }
};

/**
 *  Capture a sequence of frames that can be used to create a movie.
 *  Accepts a callback. For example, you may wish to send the frames
 *  to a server where they can be stored or converted into a movie.
 *  If no callback is provided, the browser will pop up save dialogues in an
 *  attempt to download all of the images that have just been created. With the
 *  callback provided the image data isn't saved by default but instead passed
 *  as an argument to the callback function as an array of objects, with the
 *  size of array equal to the total number of frames.
 *
 *  @method saveFrames
 *  @param  {String}   filename
 *  @param  {String}   extension 'jpg' or 'png'
 *  @param  {Number}   duration  Duration in seconds to save the frames for.
 *  @param  {Number}   framerate  Framerate to save the frames in.
 *  @param  {function(Array)} [callback] A callback function that will be executed
                                  to handle the image data. This function
                                  should accept an array as argument. The
                                  array will contain the specified number of
                                  frames of objects. Each object has three
                                  properties: imageData - an
                                  image/octet-stream, filename and extension.
 *  @example
 *  <div><code>
 *  function draw() {
 *    background(mouseX);
 *  }
 *
 *  function mousePressed() {
 *    saveFrames("out", "png", 1, 25, function(data){
 *      print(data);
 *    });
 *  }
 *  </code></div>
 *
 * @alt
 * canvas background goes from light to dark with mouse x.
 *
 */
p5.prototype.saveFrames = function(fName, ext, _duration, _fps, callback) {
  var duration = _duration || 3;
  duration = p5.prototype.constrain(duration, 0, 15);
  duration = duration * 1000;
  var fps = _fps || 15;
  fps = p5.prototype.constrain(fps, 0, 22);
  var count = 0;

  var makeFrame = p5.prototype._makeFrame;
  var cnv = this._curElement.elt;
  var frameFactory = setInterval(function(){
    makeFrame(fName + count, ext, cnv);
    count++;
  },1000/fps);

  setTimeout(function(){
    clearInterval(frameFactory);
    if (callback) {
      callback(frames);
    }
    else {
      for (var i = 0; i < frames.length; i++) {
        var f = frames[i];
        p5.prototype.downloadFile(f.imageData, f.filename, f.ext);
      }
    }
    frames = []; // clear frames
  }, duration + 0.01);
};

p5.prototype._makeFrame = function(filename, extension, _cnv) {
  var cnv;
  if (this) {
    cnv = this._curElement.elt;
  } else {
    cnv = _cnv;
  }
  var mimeType;
  if (!extension) {
    extension = 'png';
    mimeType = 'image/png';
  }
  else {
    switch(extension.toLowerCase()){
      case 'png':
        mimeType = 'image/png';
        break;
      case 'jpeg':
        mimeType = 'image/jpeg';
        break;
      case 'jpg':
        mimeType = 'image/jpeg';
        break;
      default:
        mimeType = 'image/png';
        break;
    }
  }
  var downloadMime = 'image/octet-stream';
  var imageData = cnv.toDataURL(mimeType);
  imageData = imageData.replace(mimeType, downloadMime);

  var thisFrame = {};
  thisFrame.imageData = imageData;
  thisFrame.filename = filename;
  thisFrame.ext = extension;
  frames.push(thisFrame);
};

module.exports = p5;

},{"../core/core":54}],75:[function(_dereq_,module,exports){
/**
 * @module Image
 * @submodule Loading & Displaying
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');
var Filters = _dereq_('./filters');
var canvas = _dereq_('../core/canvas');
var constants = _dereq_('../core/constants');

_dereq_('../core/error_helpers');

/**
 * Loads an image from a path and creates a p5.Image from it.
 * <br><br>
 * The image may not be immediately available for rendering
 * If you want to ensure that the image is ready before doing
 * anything with it, place the loadImage() call in preload().
 * You may also supply a callback function to handle the image when it's ready.
 * <br><br>
 * The path to the image should be relative to the HTML file
 * that links in your sketch. Loading an image from a URL or other
 * remote location may be blocked due to your browser's built-in
 * security.
 *
 * @method loadImage
 * @param  {String} path Path of the image to be loaded
 * @param  {function(p5.Image)} [successCallback] Function to be called once
 *                                the image is loaded. Will be passed the
 *                                p5.Image.
 * @param  {function(Event)}    [failureCallback] called with event error if
 *                                the image fails to load.
 * @return {p5.Image}             the p5.Image object
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/laDefense.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 * }
 * </code>
 * </div>
 * <div>
 * <code>
 * function setup() {
 *   // here we use a callback to display the image after loading
 *   loadImage("assets/laDefense.jpg", function(img) {
 *     image(img, 0, 0);
 *   });
 * }
 * </code>
 * </div>
 *
 * @alt
 * image of the underside of a white umbrella and grided ceililng above
 * image of the underside of a white umbrella and grided ceililng above
 *
 */
p5.prototype.loadImage = function(path, successCallback, failureCallback) {
  var img = new Image();
  var pImg = new p5.Image(1, 1, this);

  var self = this;
  img.onload = function() {
    pImg.width = pImg.canvas.width = img.width;
    pImg.height = pImg.canvas.height = img.height;

    // Draw the image into the backing canvas of the p5.Image
    pImg.drawingContext.drawImage(img, 0, 0);

    if (typeof successCallback === 'function') {
      successCallback(pImg);
    }

    self._decrementPreload();
  };
  img.onerror = function(e) {
    p5._friendlyFileLoadError(0,img.src);
    if (typeof failureCallback === 'function') {
      failureCallback(e);
    }
  };

  //set crossOrigin in case image is served which CORS headers
  //this will let us draw to canvas without tainting it.
  //see https://developer.mozilla.org/en-US/docs/HTML/CORS_Enabled_Image
  // When using data-uris the file will be loaded locally
  // so we don't need to worry about crossOrigin with base64 file types
  if(path.indexOf('data:image/') !== 0) {
    img.crossOrigin = 'Anonymous';
  }

  //start loading the image
  img.src = path;

  return pImg;
};

/**
 * Validates clipping params. Per drawImage spec sWidth and sHight cannot be
 * negative or greater than image intrinsic width and height
 * @private
 * @param {Number} sVal
 * @param {Number} iVal
 * @returns {Number}
 * @private
 */
function _sAssign(sVal, iVal) {
  if (sVal > 0 && sVal < iVal) {
    return sVal;
  }
  else {
    return iVal;
  }
}

/**
 * Draw an image to the main canvas of the p5js sketch
 *
 * @method image
 * @param  {p5.Image} img    the image to display
 * @param  {Number}   x      the x-coordinate at which to place the top-left
 *                           corner of the source image
 * @param  {Number}   y      the y-coordinate at which to place the top-left
 *                           corner of the source image
 * @param  {Number}   [width]  the width to draw the image
 * @param  {Number}   [height] the height to draw the image
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/laDefense.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   image(img, 0, 0, 100, 100);
 *   image(img, 0, 0, 100, 100, 0, 0, 100, 100);
 * }
 * </code>
 * </div>
 * <div>
 * <code>
 * function setup() {
 *   // here we use a callback to display the image after loading
 *   loadImage("assets/laDefense.jpg", function(img) {
 *     image(img, 0, 0);
 *   });
 * }
 * </code>
 * </div>
 *
 * @alt
 * image of the underside of a white umbrella and grided ceiling above
 * image of the underside of a white umbrella and grided ceiling above
 *
 */
/**
 * @method image
 * @param  {p5.Image} img
 * @param  {Number}   x
 * @param  {Number}   y
 * @param  {Number}   width
 * @param  {Number}   height
 * @param  {Number}   sx     the x-coordinate of the top left corner of the
 *                           sub-rectangle of the source image to draw into
 *                           the destination canvas
 * @param  {Number}   sy     the y-coordinate of the top left corner of the
 *                           sub-rectangle of the source image to draw into
 *                           the destination canvas
 * @param {Number}    [sWidth] the width of the sub-rectangle of the
 *                           source image to draw into the destination
 *                           canvas
 * @param {Number}    [sHeight] the height of the sub-rectangle of the
 *                            source image to draw into the destination context
 */
p5.prototype.image =
  function(img, dx, dy, dWidth, dHeight, sx, sy, sWidth, sHeight) {
  // set defaults per spec: https://goo.gl/3ykfOq

  var defW = img.width;
  var defH = img.height;

  if (img.elt && img.elt.videoWidth && !img.canvas) { // video no canvas
    defW = img.elt.videoWidth;
    defH = img.elt.videoHeight;
  }

  var _dx = dx;
  var _dy = dy;
  var _dw = dWidth || defW;
  var _dh = dHeight || defH;
  var _sx = sx || 0;
  var _sy = sy || 0;
  var _sw = sWidth || defW;
  var _sh = sHeight || defH;

  _sw = _sAssign(_sw, defW);
  _sh = _sAssign(_sh, defH);


  // This part needs cleanup and unit tests
  // see issues https://github.com/processing/p5.js/issues/1741
  // and https://github.com/processing/p5.js/issues/1673
  var pd = 1;

  if (img.elt && !img.canvas && img.elt.style.width) {
    //if img is video and img.elt.size() has been used and
    //no width passed to image()
    if(img.elt.videoWidth && !dWidth){
      pd = img.elt.videoWidth;
    }
    //all other cases
    else {
      pd = img.elt.width;
    }
    pd /= parseInt(img.elt.style.width, 10);
  }

  _sx *= pd;
  _sy *= pd;
  _sh *= pd;
  _sw *= pd;

  var vals = canvas.modeAdjust(_dx, _dy, _dw, _dh,
    this._renderer._imageMode);

  // tint the image if there is a tint
  this._renderer.image(img, _sx, _sy, _sw, _sh, vals.x, vals.y, vals.w,
    vals.h);
};


/**
 * Sets the fill value for displaying images. Images can be tinted to
 * specified colors or made transparent by including an alpha value.
 * <br><br>
 * To apply transparency to an image without affecting its color, use
 * white as the tint color and specify an alpha value. For instance,
 * tint(255, 128) will make an image 50% transparent (assuming the default
 * alpha range of 0-255, which can be changed with colorMode()).
 * <br><br>
 * The value for the gray parameter must be less than or equal to the current
 * maximum value as specified by colorMode(). The default maximum value is
 * 255.
 *
 *
 * @method tint
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 */

/**
 * @method tint
 * @param  {String}        value   a color string
 * @param  {Number}        [alpha]
 */

/**
 * @method tint
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 */

/**
 * @method tint
 * @param  {p5.Color}      color   the tint color
 * @param  {Number}        [alpha]
 *
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/laDefense.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   tint(0, 153, 204);  // Tint blue
 *   image(img, 50, 0);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/laDefense.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   tint(0, 153, 204, 126);  // Tint blue and set transparency
 *   image(img, 50, 0);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/laDefense.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   tint(255, 126);  // Apply transparency without changing color
 *   image(img, 50, 0);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 2 side by side images of umbrella and ceiling, one image with blue tint
 * Images of umbrella and ceiling, one half of image with blue tint
 * 2 side by side images of umbrella and ceiling, one image translucent
 *
 */
p5.prototype.tint = function () {
  var c = this.color.apply(this, arguments);
  this._renderer._tint = c.levels;
};

/**
 * Removes the current fill value for displaying images and reverts to
 * displaying images with their original hues.
 *
 * @method noTint
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *   tint(0, 153, 204);  // Tint blue
 *   image(img, 0, 0);
 *   noTint();  // Disable tint
 *   image(img, 50, 0);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 2 side by side images of bricks, left image with blue tint
 *
 */
p5.prototype.noTint = function() {
  this._renderer._tint = null;
};

/**
 * Apply the current tint color to the input image, return the resulting
 * canvas.
 *
 * @param {p5.Image} The image to be tinted
 * @return {canvas} The resulting tinted canvas
 *
 */
p5.prototype._getTintedImageCanvas = function(img) {
  if (!img.canvas) {
    return img;
  }
  var pixels = Filters._toPixels(img.canvas);
  var tmpCanvas = document.createElement('canvas');
  tmpCanvas.width = img.canvas.width;
  tmpCanvas.height = img.canvas.height;
  var tmpCtx = tmpCanvas.getContext('2d');
  var id = tmpCtx.createImageData(img.canvas.width, img.canvas.height);
  var newPixels = id.data;

  for(var i = 0; i < pixels.length; i += 4) {
    var r = pixels[i];
    var g = pixels[i+1];
    var b = pixels[i+2];
    var a = pixels[i+3];

    newPixels[i] = r*this._renderer._tint[0]/255;
    newPixels[i+1] = g*this._renderer._tint[1]/255;
    newPixels[i+2] = b*this._renderer._tint[2]/255;
    newPixels[i+3] = a*this._renderer._tint[3]/255;
  }

  tmpCtx.putImageData(id, 0, 0);
  return tmpCanvas;
};

/**
 * Set image mode. Modifies the location from which images are drawn by
 * changing the way in which parameters given to image() are interpreted.
 * The default mode is imageMode(CORNER), which interprets the second and
 * third parameters of image() as the upper-left corner of the image. If
 * two additional parameters are specified, they are used to set the image's
 * width and height.
 * <br><br>
 * imageMode(CORNERS) interprets the second and third parameters of image()
 * as the location of one corner, and the fourth and fifth parameters as the
 * opposite corner.
 * <br><br>
 * imageMode(CENTER) interprets the second and third parameters of image()
 * as the image's center point. If two additional parameters are specified,
 * they are used to set the image's width and height.
 *
 * @method imageMode
 * @param {Constant} mode either CORNER, CORNERS, or CENTER
 * @example
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *   imageMode(CORNER);
 *   image(img, 10, 10, 50, 50);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *   imageMode(CORNERS);
 *   image(img, 10, 10, 90, 40);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *   imageMode(CENTER);
 *   image(img, 50, 50, 80, 80);
 * }
 * </code>
 * </div>
 *
 * @alt
 * small square image of bricks
 * horizontal rectangle image of bricks
 * large square image of bricks
 *
 */
p5.prototype.imageMode = function(m) {
  if (m === constants.CORNER ||
    m === constants.CORNERS ||
    m === constants.CENTER) {
    this._renderer._imageMode = m;
  }
};


module.exports = p5;

},{"../core/canvas":52,"../core/constants":53,"../core/core":54,"../core/error_helpers":57,"./filters":73}],76:[function(_dereq_,module,exports){
/**
 * @module Image
 * @submodule Image
 * @requires core
 * @requires constants
 * @requires filters
 */

/**
 * This module defines the p5.Image class and P5 methods for
 * drawing images to the main display canvas.
 */

'use strict';

var p5 = _dereq_('../core/core');
var Filters = _dereq_('./filters');

/*
 * Class methods
 */

/**
 * Creates a new p5.Image. A p5.Image is a canvas backed representation of an
 * image.
 * <br><br>
 * p5 can display .gif, .jpg and .png images. Images may be displayed
 * in 2D and 3D space. Before an image is used, it must be loaded with the
 * loadImage() function. The p5.Image class contains fields for the width and
 * height of the image, as well as an array called pixels[] that contains the
 * values for every pixel in the image.
 * <br><br>
 * The methods described below allow easy access to the image's pixels and
 * alpha channel and simplify the process of compositing.
 * <br><br>
 * Before using the pixels[] array, be sure to use the loadPixels() method on
 * the image to make sure that the pixel data is properly loaded.
 *
 * @class p5.Image
 * @constructor
 * @param {Number} width
 * @param {Number} height
 */
p5.Image = function(width, height){
  /**
   * Image width.
   * @property {Number} width
   * @readOnly
   * @example
   * <div><code>
   * var img;
   * function preload() {
   *   img = loadImage("assets/rockies.jpg");
   * }
   *
   * function setup() {
   *   createCanvas(100, 100);
   *   image(img, 0, 0);
   *   for (var i=0; i < img.width; i++) {
   *     var c = img.get(i, img.height/2);
   *     stroke(c);
   *     line(i, height/2, i, height);
   *   }
   * }
   * </code></div>
   *
   * @alt
   * rocky mountains in top and horizontal lines in corresponding colors in bottom.
   *
   */
  this.width = width;
  /**
   * Image height.
   * @property {Number} height
   * @readOnly
   * @example
   * <div><code>
   * var img;
   * function preload() {
   *   img = loadImage("assets/rockies.jpg");
   * }
   *
   * function setup() {
   *   createCanvas(100, 100);
   *   image(img, 0, 0);
   *   for (var i=0; i < img.height; i++) {
   *     var c = img.get(img.width/2, i);
   *     stroke(c);
   *     line(0, i, width/2, i);
   *   }
   * }
   * </code></div>
   *
   * @alt
   * rocky mountains on right and vertical lines in corresponding colors on left.
   *
   */
  this.height = height;
  this.canvas = document.createElement('canvas');
  this.canvas.width = this.width;
  this.canvas.height = this.height;
  this.drawingContext = this.canvas.getContext('2d');
  this._pixelDensity = 1;
  //used for webgl texturing only
  this.isTexture = false;
  /**
   * Array containing the values for all the pixels in the display window.
   * These values are numbers. This array is the size (include an appropriate
   * factor for pixelDensity) of the display window x4,
   * representing the R, G, B, A values in order for each pixel, moving from
   * left to right across each row, then down each column. Retina and other
   * high denisty displays may have more pixels[] (by a factor of
   * pixelDensity^2).
   * For example, if the image is 100x100 pixels, there will be 40,000. With
   * pixelDensity = 2, there will be 160,000. The first four values
   * (indices 0-3) in the array will be the R, G, B, A values of the pixel at
   * (0, 0). The second four values (indices 4-7) will contain the R, G, B, A
   * values of the pixel at (1, 0). More generally, to set values for a pixel
   * at (x, y):
   * ```javascript
   * var d = pixelDensity;
   * for (var i = 0; i < d; i++) {
   *   for (var j = 0; j < d; j++) {
   *     // loop over
   *     idx = 4 * ((y * d + j) * width * d + (x * d + i));
   *     pixels[idx] = r;
   *     pixels[idx+1] = g;
   *     pixels[idx+2] = b;
   *     pixels[idx+3] = a;
   *   }
   * }
   * ```
   * <br><br>
   * Before accessing this array, the data must loaded with the loadPixels()
   * function. After the array data has been modified, the updatePixels()
   * function must be run to update the changes.
   * @property {Number[]} pixels
   * @example
   * <div>
   * <code>
   * img = createImage(66, 66);
   * img.loadPixels();
   * for (i = 0; i < img.width; i++) {
   *   for (j = 0; j < img.height; j++) {
   *     img.set(i, j, color(0, 90, 102));
   *   }
   * }
   * img.updatePixels();
   * image(img, 17, 17);
   * </code>
   * </div>
   * <div>
   * <code>
   * var pink = color(255, 102, 204);
   * img = createImage(66, 66);
   * img.loadPixels();
   * for (var i = 0; i < 4*(width*height/2); i+=4) {
   *   img.pixels[i] = red(pink);
   *   img.pixels[i+1] = green(pink);
   *   img.pixels[i+2] = blue(pink);
   *   img.pixels[i+3] = alpha(pink);
   * }
   * img.updatePixels();
   * image(img, 17, 17);
   * </code>
   * </div>
   *
   * @alt
   * 66x66 turquoise rect in center of canvas
   * 66x66 pink rect in center of canvas
   *
   */
  this.pixels = [];
};

/**
 * Helper fxn for sharing pixel methods
 *
 */
p5.Image.prototype._setProperty = function (prop, value) {
  this[prop] = value;
};

/**
 * Loads the pixels data for this image into the [pixels] attribute.
 *
 * @method loadPixels
 * @example
 * <div><code>
 * var myImage;
 * var halfImage;
 *
 * function preload() {
 *   myImage = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   myImage.loadPixels();
 *   halfImage = 4 * width * height/2;
 *   for(var i = 0; i < halfImage; i++){
 *     myImage.pixels[i+halfImage] = myImage.pixels[i];
 *   }
 *   myImage.updatePixels();
 * }
 *
 * function draw() {
 *   image(myImage, 0, 0);
 * }
 * </code></div>
 *
   * @alt
   * 2 images of rocky mountains vertically stacked
   *
 */
p5.Image.prototype.loadPixels = function(){
  p5.Renderer2D.prototype.loadPixels.call(this);
};

/**
 * Updates the backing canvas for this image with the contents of
 * the [pixels] array.
 *
 * @method updatePixels
 * @param {Integer} x x-offset of the target update area for the
 *                              underlying canvas
 * @param {Integer} y y-offset of the target update area for the
 *                              underlying canvas
 * @param {Integer} w height of the target update area for the
 *                              underlying canvas
 * @param {Integer} h height of the target update area for the
 *                              underlying canvas
 */
/**
 * @method updatePixels
 * @example
 * <div><code>
 * var myImage;
 * var halfImage;
 *
 * function preload() {
 *   myImage = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   myImage.loadPixels();
 *   halfImage = 4 * width * height/2;
 *   for(var i = 0; i < halfImage; i++){
 *     myImage.pixels[i+halfImage] = myImage.pixels[i];
 *   }
 *   myImage.updatePixels();
 * }
 *
 * function draw() {
 *   image(myImage, 0, 0);
 * }
 * </code></div>
 *
 * @alt
 * 2 images of rocky mountains vertically stacked
 *
 */
p5.Image.prototype.updatePixels = function(x, y, w, h){
  p5.Renderer2D.prototype.updatePixels.call(this, x, y, w, h);
};

/**
 * Get a region of pixels from an image.
 *
 * If no params are passed, those whole image is returned,
 * if x and y are the only params passed a single pixel is extracted
 * if all params are passed a rectangle region is extracted and a p5.Image
 * is returned.
 *
 * Returns undefined if the region is outside the bounds of the image
 *
 * @method get
 * @param  {Number}               [x] x-coordinate of the pixel
 * @param  {Number}               [y] y-coordinate of the pixel
 * @param  {Number}               [w] width
 * @param  {Number}               [h] height
 * @return {Number[]|Color|p5.Image}  color of pixel at x,y in array format
 *                                    [R, G, B, A] or p5.Image
 * @example
 * <div><code>
 * var myImage;
 * var c;
 *
 * function preload() {
 *   myImage = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   background(myImage);
 *   noStroke();
 *   c = myImage.get(60, 90);
 *   fill(c);
 *   rect(25, 25, 50, 50);
 * }
 *
 * //get() returns color here
 * </code></div>
 *
 * @alt
 * image of rocky mountains with 50x50 green rect in front
 *
 */
p5.Image.prototype.get = function(x, y, w, h){
  return p5.Renderer2D.prototype.get.call(this, x, y, w, h);
};

/**
 * Set the color of a single pixel or write an image into
 * this p5.Image.
 *
 * Note that for a large number of pixels this will
 * be slower than directly manipulating the pixels array
 * and then calling updatePixels().
 *
 * @method set
 * @param {Number}              x x-coordinate of the pixel
 * @param {Number}              y y-coordinate of the pixel
 * @param {Number|Array|Object}   a grayscale value | pixel array |
 *                                a p5.Color | image to copy
 * @example
 * <div>
 * <code>
 * img = createImage(66, 66);
 * img.loadPixels();
 * for (i = 0; i < img.width; i++) {
 *   for (j = 0; j < img.height; j++) {
 *     img.set(i, j, color(0, 90, 102, i % img.width * 2));
 *   }
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 * image(img, 34, 34);
 * </code>
 * </div>
 *
 * @alt
 * 2 gradated dark turquoise rects fade left. 1 center 1 bottom right of canvas
 *
 */
p5.Image.prototype.set = function(x, y, imgOrCol){
  p5.Renderer2D.prototype.set.call(this, x, y, imgOrCol);
};

/**
 * Resize the image to a new width and height. To make the image scale
 * proportionally, use 0 as the value for the wide or high parameter.
 * For instance, to make the width of an image 150 pixels, and change
 * the height using the same proportion, use resize(150, 0).
 *
 * @method resize
 * @param {Number} width the resized image width
 * @param {Number} height the resized image height
 * @example
 * <div><code>
 * var img;
 *
 * function setup() {
 *   img = loadImage("assets/rockies.jpg");
 * }

 * function draw() {
 *   image(img, 0, 0);
 * }
 *
 * function mousePressed() {
 *   img.resize(50, 100);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains. zoomed in
 *
 */
p5.Image.prototype.resize = function(width, height){

  // Copy contents to a temporary canvas, resize the original
  // and then copy back.
  //
  // There is a faster approach that involves just one copy and swapping the
  // this.canvas reference. We could switch to that approach if (as i think
  // is the case) there an expectation that the user would not hold a
  // reference to the backing canvas of a p5.Image. But since we do not
  // enforce that at the moment, I am leaving in the slower, but safer
  // implementation.

  // auto-resize
  if (width === 0 && height === 0) {
    width = this.canvas.width;
    height = this.canvas.height;
  } else if (width === 0) {
    width = this.canvas.width * height / this.canvas.height;
  } else if (height === 0) {
    height = this.canvas.height * width / this.canvas.width;
  }

  width = Math.floor(width);
  height = Math.floor(height);

  var tempCanvas = document.createElement('canvas');
  tempCanvas.width = width;
  tempCanvas.height = height;
  tempCanvas.getContext('2d').drawImage(this.canvas,
    0, 0, this.canvas.width, this.canvas.height,
    0, 0, tempCanvas.width, tempCanvas.height
  );


  // Resize the original canvas, which will clear its contents
  this.canvas.width = this.width = width;
  this.canvas.height = this.height = height;

  //Copy the image back

  this.drawingContext.drawImage(tempCanvas,
    0, 0, width, height,
    0, 0, width, height
  );

  if(this.pixels.length > 0){
    this.loadPixels();
  }
};

/**
 * Copies a region of pixels from one image to another. If no
 * srcImage is specified this is used as the source. If the source
 * and destination regions aren't the same size, it will
 * automatically resize source pixels to fit the specified
 * target region.
 *
 * @method copy
 * @param  {p5.Image|undefined} srcImage source image
 * @param  {Integer} sx X coordinate of the source's upper left corner
 * @param  {Integer} sy Y coordinate of the source's upper left corner
 * @param  {Integer} sw source image width
 * @param  {Integer} sh source image height
 * @param  {Integer} dx X coordinate of the destination's upper left corner
 * @param  {Integer} dy Y coordinate of the destination's upper left corner
 * @param  {Integer} dw destination image width
 * @param  {Integer} dh destination image height
 * @example
 * <div><code>
 * var photo;
 * var bricks;
 * var x;
 * var y;
 *
 * function preload() {
 *   photo = loadImage("assets/rockies.jpg");
 *   bricks = loadImage("assets/bricks.jpg");
 * }
 *
 * function setup() {
 *   x = bricks.width/2;
 *   y = bricks.height/2;
 *   photo.copy(bricks, 0, 0, x, y, 0, 0, x, y);
 *   image(photo, 0, 0);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains and smaller image on top of bricks at top left
 *
 */
p5.Image.prototype.copy = function () {
  p5.prototype.copy.apply(this, arguments);
};

/**
 * Masks part of an image from displaying by loading another
 * image and using it's alpha channel as an alpha channel for
 * this image.
 *
 * @method mask
 * @param {p5.Image} srcImage source image
 * @example
 * <div><code>
 * var photo, maskImage;
 * function preload() {
 *   photo = loadImage("assets/rockies.jpg");
 *   maskImage = loadImage("assets/mask2.png");
 * }
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   photo.mask(maskImage);
 *   image(photo, 0, 0);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains with white at right
 *
 *
 * http://blogs.adobe.com/webplatform/2013/01/28/blending-features-in-canvas/
 *
 */
// TODO: - Accept an array of alpha values.
//       - Use other channels of an image. p5 uses the
//       blue channel (which feels kind of arbitrary). Note: at the
//       moment this method does not match native processings original
//       functionality exactly.
p5.Image.prototype.mask = function(p5Image) {
  if(p5Image === undefined){
    p5Image = this;
  }
  var currBlend = this.drawingContext.globalCompositeOperation;

  var scaleFactor = 1;
  if (p5Image instanceof p5.Renderer) {
    scaleFactor = p5Image._pInst._pixelDensity;
  }

  var copyArgs = [
    p5Image,
    0,
    0,
    scaleFactor*p5Image.width,
    scaleFactor*p5Image.height,
    0,
    0,
    this.width,
    this.height
  ];

  this.drawingContext.globalCompositeOperation = 'destination-in';
  p5.Image.prototype.copy.apply(this, copyArgs);
  this.drawingContext.globalCompositeOperation = currBlend;
};

/**
 * Applies an image filter to a p5.Image
 *
 * @method filter
 * @param {String} operation one of threshold, gray, invert, posterize, opaque
 *                           erode, dilate and blur. See Filters.js for docs on
 *                           each available filter
 * @param {Number|undefined} value
 * @example
 * <div><code>
 * var photo1;
 * var photo2;
 *
 * function preload() {
 *   photo1 = loadImage("assets/rockies.jpg");
 *   photo2 = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   photo2.filter("gray");
 *   image(photo1, 0, 0);
 *   image(photo2, width/2, 0);
 * }
 * </code></div>
 *
 * @alt
 * 2 images of rocky mountains left one in color, right in black and white
 *
 */
p5.Image.prototype.filter = function(operation, value) {
  Filters.apply(this.canvas, Filters[operation.toLowerCase()], value);
};

/**
 * Copies a region of pixels from one image to another, using a specified
 * blend mode to do the operation.
 *
 * @method blend
 * @param  {p5.Image|undefined} srcImage source image
 * @param  {Integer} sx X coordinate of the source's upper left corner
 * @param  {Integer} sy Y coordinate of the source's upper left corner
 * @param  {Integer} sw source image width
 * @param  {Integer} sh source image height
 * @param  {Integer} dx X coordinate of the destination's upper left corner
 * @param  {Integer} dy Y coordinate of the destination's upper left corner
 * @param  {Integer} dw destination image width
 * @param  {Integer} dh destination image height
 * @param  {Integer} blendMode the blend mode
 *
 * Available blend modes are: normal | multiply | screen | overlay |
 *            darken | lighten | color-dodge | color-burn | hard-light |
 *            soft-light | difference | exclusion | hue | saturation |
 *            color | luminosity
 *
 *
 * http://blogs.adobe.com/webplatform/2013/01/28/blending-features-in-canvas/
 * @example
 * <div><code>
 * var mountains;
 * var bricks;
 *
 * function preload() {
 *   mountains = loadImage("assets/rockies.jpg");
 *   bricks = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   mountains.blend(bricks, 0, 0, 33, 100, 67, 0, 33, 100, ADD);
 *   image(mountains, 0, 0);
 *   image(bricks, 0, 0);
 * }
 * </code></div>
 * <div><code>
 * var mountains;
 * var bricks;
 *
 * function preload() {
 *   mountains = loadImage("assets/rockies.jpg");
 *   bricks = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   mountains.blend(bricks, 0, 0, 33, 100, 67, 0, 33, 100, DARKEST);
 *   image(mountains, 0, 0);
 *   image(bricks, 0, 0);
 * }
 * </code></div>
 * <div><code>
 * var mountains;
 * var bricks;
 *
 * function preload() {
 *   mountains = loadImage("assets/rockies.jpg");
 *   bricks = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   mountains.blend(bricks, 0, 0, 33, 100, 67, 0, 33, 100, LIGHTEST);
 *   image(mountains, 0, 0);
 *   image(bricks, 0, 0);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains. Brick images on left and right. Right overexposed
 * image of rockies. Brickwall images on left and right. Right mortar transparent
 * image of rockies. Brickwall images on left and right. Right translucent
 *
 */
p5.Image.prototype.blend = function() {
  p5.prototype.blend.apply(this, arguments);
};

/**
 * Saves the image to a file and force the browser to download it.
 * Accepts two strings for filename and file extension
 * Supports png (default) and jpg.
 *
 * @method save
 * @param {String} filename give your file a name
 * @param  {String} extension 'png' or 'jpg'
 * @example
 * <div><code>
 * var photo;
 *
 * function preload() {
 *   photo = loadImage("assets/rockies.jpg");
 * }
 *
 * function draw() {
 *   image(photo, 0, 0);
 * }
 *
 * function keyTyped() {
 *   if (key == 's') {
 *     photo.save("photo", "png");
 *   }
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains.
 *
 */
p5.Image.prototype.save = function(filename, extension) {
  var mimeType;
  if (!extension) {
    extension = 'png';
    mimeType = 'image/png';
  }
  else {
    // en.wikipedia.org/wiki/Comparison_of_web_browsers#Image_format_support
    switch(extension.toLowerCase()){
      case 'png':
        mimeType = 'image/png';
        break;
      case 'jpeg':
        mimeType = 'image/jpeg';
        break;
      case 'jpg':
        mimeType = 'image/jpeg';
        break;
      default:
        mimeType = 'image/png';
        break;
    }
  }
  var downloadMime = 'image/octet-stream';
  var imageData = this.canvas.toDataURL(mimeType);
  imageData = imageData.replace(mimeType, downloadMime);

  //Make the browser download the file
  p5.prototype.downloadFile(imageData, filename, extension);
};

module.exports = p5.Image;
},{"../core/core":54,"./filters":73}],77:[function(_dereq_,module,exports){
/**
 * @module Image
 * @submodule Pixels
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');
var Filters = _dereq_('./filters');
_dereq_('../color/p5.Color');

/**
 * <a href='https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference
 * /Global_Objects/Uint8ClampedArray' target='_blank'>Uint8ClampedArray</a>
 * containing the values for all the pixels in the display window.
 * These values are numbers. This array is the size (include an appropriate
 * factor for pixelDensity) of the display window x4,
 * representing the R, G, B, A values in order for each pixel, moving from
 * left to right across each row, then down each column. Retina and other
 * high density displays will have more pixels[] (by a factor of
 * pixelDensity^2).
 * For example, if the image is 100x100 pixels, there will be 40,000. On a
 * retina display, there will be 160,000.
 * <br><br>
 * The first four values (indices 0-3) in the array will be the R, G, B, A
 * values of the pixel at (0, 0). The second four values (indices 4-7) will
 * contain the R, G, B, A values of the pixel at (1, 0). More generally, to
 * set values for a pixel at (x, y):
 * ```javascript
 * var d = pixelDensity;
 * for (var i = 0; i < d; i++) {
 *   for (var j = 0; j < d; j++) {
 *     // loop over
 *     idx = 4 * ((y * d + j) * width * d + (x * d + i));
 *     pixels[idx] = r;
 *     pixels[idx+1] = g;
 *     pixels[idx+2] = b;
 *     pixels[idx+3] = a;
 *   }
 * }
 * ```
 * <p>While the above method is complex, it is flexible enough to work with
 * any pixelDensity. Note that set() will automatically take care of
 * setting all the appropriate values in pixels[] for a given (x, y) at
 * any pixelDensity, but the performance may not be as fast when lots of
 * modifications are made to the pixel array.
 * <br><br>
 * Before accessing this array, the data must loaded with the loadPixels()
 * function. After the array data has been modified, the updatePixels()
 * function must be run to update the changes.
 * <br><br>
 * Note that this is not a standard javascript array.  This means that
 * standard javascript functions such as <code>slice()</code> or
 * <code>arrayCopy()</code> do not
 * work.</p>
 *
 * @property {Number[]} pixels
 * @example
 * <div>
 * <code>
 * var pink = color(255, 102, 204);
 * loadPixels();
 * var d = pixelDensity();
 * var halfImage = 4 * (width * d) * (height/2 * d);
 * for (var i = 0; i < halfImage; i+=4) {
 *   pixels[i] = red(pink);
 *   pixels[i+1] = green(pink);
 *   pixels[i+2] = blue(pink);
 *   pixels[i+3] = alpha(pink);
 * }
 * updatePixels();
 * </code>
 * </div>
 *
 * @alt
 * top half of canvas pink, bottom grey
 *
 */
p5.prototype.pixels = [];

/**
 * Copies a region of pixels from one image to another, using a specified
 * blend mode to do the operation.<br><br>
 * Available blend modes are: BLEND | DARKEST | LIGHTEST | DIFFERENCE |
 * MULTIPLY| EXCLUSION | SCREEN | REPLACE | OVERLAY | HARD_LIGHT |
 * SOFT_LIGHT | DODGE | BURN | ADD | NORMAL
 *
 *
 * @method blend
 * @param  {p5.Image|undefined} srcImage source image
 * @param  {Integer} sx X coordinate of the source's upper left corner
 * @param  {Integer} sy Y coordinate of the source's upper left corner
 * @param  {Integer} sw source image width
 * @param  {Integer} sh source image height
 * @param  {Integer} dx X coordinate of the destination's upper left corner
 * @param  {Integer} dy Y coordinate of the destination's upper left corner
 * @param  {Integer} dw destination image width
 * @param  {Integer} dh destination image height
 * @param  {Integer} blendMode the blend mode
 *
 * @example
 * <div><code>
 * var img0;
 * var img1;
 *
 * function preload() {
 *   img0 = loadImage("assets/rockies.jpg");
 *   img1 = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   background(img0);
 *   image(img1, 0, 0);
 *   blend(img1, 0, 0, 33, 100, 67, 0, 33, 100, LIGHTEST);
 * }
 * </code></div>
 * <div><code>
 * var img0;
 * var img1;
 *
 * function preload() {
 *   img0 = loadImage("assets/rockies.jpg");
 *   img1 = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   background(img0);
 *   image(img1, 0, 0);
 *   blend(img1, 0, 0, 33, 100, 67, 0, 33, 100, DARKEST);
 * }
 * </code></div>
 * <div><code>
 * var img0;
 * var img1;
 *
 * function preload() {
 *   img0 = loadImage("assets/rockies.jpg");
 *   img1 = loadImage("assets/bricks_third.jpg");
 * }
 *
 * function setup() {
 *   background(img0);
 *   image(img1, 0, 0);
 *   blend(img1, 0, 0, 33, 100, 67, 0, 33, 100, ADD);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains. Brick images on left and right. Right overexposed
 * image of rockies. Brickwall images on left and right. Right mortar transparent
 * image of rockies. Brickwall images on left and right. Right translucent
 *
 *
 */
p5.prototype.blend = function() {
  if (this._renderer) {
    this._renderer.blend.apply(this._renderer, arguments);
  } else {
    p5.Renderer2D.prototype.blend.apply(this, arguments);
  }
};

/**
 * Copies a region of the canvas to another region of the canvas
 * and copies a region of pixels from an image used as the srcImg parameter
 * into the canvas srcImage is specified this is used as the source. If
 * the source and destination regions aren't the same size, it will
 * automatically resize source pixels to fit the specified
 * target region.
 *
 * @method copy
 * @param  {p5.Image|undefined} srcImage source image
 * @param  {Integer} sx X coordinate of the source's upper left corner
 * @param  {Integer} sy Y coordinate of the source's upper left corner
 * @param  {Integer} sw source image width
 * @param  {Integer} sh source image height
 * @param  {Integer} dx X coordinate of the destination's upper left corner
 * @param  {Integer} dy Y coordinate of the destination's upper left corner
 * @param  {Integer} dw destination image width
 * @param  {Integer} dh destination image height
 *
 * @example
 * <div><code>
 * var img;
 *
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   background(img);
 *   copy(img, 7, 22, 10, 10, 35, 25, 50, 50);
 *   stroke(255);
 *   noFill();
 *   // Rectangle shows area being copied
 *   rect(7, 22, 10, 10);
 * }
 * </code></div>
 *
 * @alt
 * image of rocky mountains. Brick images on left and right. Right overexposed
 * image of rockies. Brickwall images on left and right. Right mortar transparent
 * image of rockies. Brickwall images on left and right. Right translucent
 *
 */
p5.prototype.copy = function () {
  p5.Renderer2D._copyHelper.apply(this, arguments);
};

/**
 * Applies a filter to the canvas.
 * <br><br>
 *
 * The presets options are:
 * <br><br>
 *
 * THRESHOLD
 * Converts the image to black and white pixels depending if they are above or
 * below the threshold defined by the level parameter. The parameter must be
 * between 0.0 (black) and 1.0 (white). If no level is specified, 0.5 is used.
 * <br><br>
 *
 * GRAY
 * Converts any colors in the image to grayscale equivalents. No parameter
 * is used.
 * <br><br>
 *
 * OPAQUE
 * Sets the alpha channel to entirely opaque. No parameter is used.
 * <br><br>
 *
 * INVERT
 * Sets each pixel to its inverse value. No parameter is used.
 * <br><br>
 *
 * POSTERIZE
 * Limits each channel of the image to the number of colors specified as the
 * parameter. The parameter can be set to values between 2 and 255, but
 * results are most noticeable in the lower ranges.
 * <br><br>
 *
 * BLUR
 * Executes a Guassian blur with the level parameter specifying the extent
 * of the blurring. If no parameter is used, the blur is equivalent to
 * Guassian blur of radius 1. Larger values increase the blur.
 * <br><br>
 *
 * ERODE
 * Reduces the light areas. No parameter is used.
 * <br><br>
 *
 * DILATE
 * Increases the light areas. No parameter is used.
 *
 * @method filter
 * @param  {Constant} filterType either THRESHOLD, GRAY, OPAQUE, INVERT,
 *                                POSTERIZE, BLUR, ERODE or DILATE
 * @param  {Number} filterParam an optional parameter unique
 *  to each filter, see above
 *
 *
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(THRESHOLD);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(GRAY);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(OPAQUE);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(INVERT);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(POSTERIZE,3);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(DILATE);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(BLUR,3);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/bricks.jpg");
 * }
 * function setup() {
 *  image(img, 0, 0);
 *  filter(ERODE);
 * }
 * </code>
 * </div>
 *
 * @alt
 * black and white image of a brick wall.
 * greyscale image of a brickwall
 * image of a brickwall
 * jade colored image of a brickwall
 * red and pink image of a brickwall
 * image of a brickwall
 * blurry image of a brickwall
 * image of a brickwall
 * image of a brickwall with less detail
 *
 */
p5.prototype.filter = function(operation, value) {
  if(this.canvas !== undefined) {
    Filters.apply(this.canvas, Filters[operation.toLowerCase()], value);
  }
  else {
    Filters.apply(this.elt, Filters[operation.toLowerCase()], value);
  }
};

/**
 * Returns an array of [R,G,B,A] values for any pixel or grabs a section of
 * an image. If no parameters are specified, the entire image is returned.
 * Use the x and y parameters to get the value of one pixel. Get a section of
 * the display window by specifying additional w and h parameters. When
 * getting an image, the x and y parameters define the coordinates for the
 * upper-left corner of the image, regardless of the current imageMode().
 * <br><br>
 * If the pixel requested is outside of the image window, [0,0,0,255] is
 * returned. To get the numbers scaled according to the current color ranges
 * and taking into account colorMode, use getColor instead of get.
 * <br><br>
 * Getting the color of a single pixel with get(x, y) is easy, but not as fast
 * as grabbing the data directly from pixels[]. The equivalent statement to
 * get(x, y) using pixels[] with pixel density d is
 * <code>
 * var off = (y * width + x) * d * 4;
 * [pixels[off],
 * pixels[off+1],
 * pixels[off+2],
 * pixels[off+3]]</code>
 * <br><br>
 * See the reference for pixels[] for more information.
 *
 * @method get
 * @param  {Number}         [x] x-coordinate of the pixel
 * @param  {Number}         [y] y-coordinate of the pixel
 * @param  {Number}         [w] width
 * @param  {Number}         [h] height
 * @return {Number[]|p5.Image}  values of pixel at x,y in array format
 *                              [R, G, B, A] or p5.Image
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   var c = get();
 *   image(c, width/2, 0);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 * function setup() {
 *   image(img, 0, 0);
 *   var c = get(50, 90);
 *   fill(c);
 *   noStroke();
 *   rect(25, 25, 50, 50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 2 images of the rocky mountains, side-by-side
 * Image of the rocky mountains with 50x50 green rect in center of canvas
 *
 */
p5.prototype.get = function(x, y, w, h){
  return this._renderer.get(x, y, w, h);
};

/**
 * Loads the pixel data for the display window into the pixels[] array. This
 * function must always be called before reading from or writing to pixels[].
 * Note that only changes made with set() or direct manipulation of pixels[]
 * will occur.
 *
 * @method loadPixels
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   image(img, 0, 0);
 *   var d = pixelDensity();
 *   var halfImage = 4 * (img.width * d) *
       (img.height/2 * d);
 *   loadPixels();
 *   for (var i = 0; i < halfImage; i++) {
 *     pixels[i+halfImage] = pixels[i];
 *   }
 *   updatePixels();
 * }
 * </code>
 * </div>
 *
 * @alt
 * two images of the rocky mountains. one on top, one on bottom of canvas.
 *
 */
p5.prototype.loadPixels = function() {
  this._renderer.loadPixels();
};

/**
 * <p>Changes the color of any pixel, or writes an image directly to the
 * display window.</p>
 * <p>The x and y parameters specify the pixel to change and the c parameter
 * specifies the color value. This can be a p5.Color object, or [R, G, B, A]
 * pixel array. It can also be a single grayscale value.
 * When setting an image, the x and y parameters define the coordinates for
 * the upper-left corner of the image, regardless of the current imageMode().
 * </p>
 * <p>
 * After using set(), you must call updatePixels() for your changes to appear.
 * This should be called once all pixels have been set, and must be called before
 * calling .get() or drawing the image.
 * </p>
 * <p>Setting the color of a single pixel with set(x, y) is easy, but not as
 * fast as putting the data directly into pixels[]. Setting the pixels[]
 * values directly may be complicated when working with a retina display,
 * but will perform better when lots of pixels need to be set directly on
 * every loop.</p>
 * <p>See the reference for pixels[] for more information.</p>
 *
 * @method set
 * @param {Number}              x x-coordinate of the pixel
 * @param {Number}              y y-coordinate of the pixel
 * @param {Number|Array|Object} c insert a grayscale value | a pixel array |
 *                                a p5.Color object | a p5.Image to copy
 * @example
 * <div>
 * <code>
 * var black = color(0);
 * set(30, 20, black);
 * set(85, 20, black);
 * set(85, 75, black);
 * set(30, 75, black);
 * updatePixels();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * for (var i = 30; i < width-15; i++) {
 *   for (var j = 20; j < height-25; j++) {
 *     var c = color(204-j, 153-i, 0);
 *     set(i, j, c);
 *   }
 * }
 * updatePixels();
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   set(0, 0, img);
 *   updatePixels();
 *   line(0, 0, width, height);
 *   line(0, height, width, 0);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 4 black points in the shape of a square middle-right of canvas.
 * square with orangey-brown gradient lightening at bottom right.
 * image of the rocky mountains. with lines like an 'x' through the center.
 */
p5.prototype.set = function (x, y, imgOrCol) {
  this._renderer.set(x, y, imgOrCol);
};
/**
 * Updates the display window with the data in the pixels[] array.
 * Use in conjunction with loadPixels(). If you're only reading pixels from
 * the array, there's no need to call updatePixels() — updating is only
 * necessary to apply changes. updatePixels() should be called anytime the
 * pixels array is manipulated or set() is called, and only changes made with
 * set() or direct changes to pixels[] will occur.
 *
 * @method updatePixels
 * @param  {Number} [x]    x-coordinate of the upper-left corner of region
 *                         to update
 * @param  {Number} [y]    y-coordinate of the upper-left corner of region
 *                         to update
 * @param  {Number} [w]    width of region to update
 * @param  {Number} [h]    height of region to update
 * @example
 * <div>
 * <code>
 * var img;
 * function preload() {
 *   img = loadImage("assets/rockies.jpg");
 * }
 *
 * function setup() {
 *   image(img, 0, 0);
 *   var halfImage = 4 * (img.width * pixelDensity()) *
 *     (img.height * pixelDensity()/2);
 *   loadPixels();
 *   for (var i = 0; i < halfImage; i++) {
 *     pixels[i+halfImage] = pixels[i];
 *   }
 *   updatePixels();
 * }
 * </code>
 * </div>
 * @alt
 * two images of the rocky mountains. one on top, one on bottom of canvas.
 */
p5.prototype.updatePixels = function (x, y, w, h) {
  // graceful fail - if loadPixels() or set() has not been called, pixel
  // array will be empty, ignore call to updatePixels()
  if (this.pixels.length === 0) {
    return;
  }
  this._renderer.updatePixels(x, y, w, h);
};

module.exports = p5;

},{"../color/p5.Color":48,"../core/core":54,"./filters":73}],78:[function(_dereq_,module,exports){
/**
 * @module IO
 * @submodule Input
 * @for p5
 * @requires core
 */

/* globals Request: false */
/* globals Headers: false */

'use strict';

var p5 = _dereq_('../core/core');
_dereq_('../core/error_helpers');

/**
 * Reads the contents of a file and creates a String array of its individual
 * lines. If the name of the file is used as the parameter, as in the above
 * example, the file must be located in the sketch directory/folder.
 * <br><br>
 * Alternatively, the file maybe be loaded from anywhere on the local
 * computer using an absolute path (something that starts with / on Unix and
 * Linux, or a drive letter on Windows), or the filename parameter can be a
 * URL for a file found on a network.
 * <br><br>
 * This method is asynchronous, meaning it may not finish before the next
 * line in your sketch is executed.
 *
 * @method loadStrings
 * @param  {String}   filename   name of the file or url to load
 * @param  {function} [callback] function to be executed after loadStrings()
 *                               completes, Array is passed in as first
 *                               argument
 * @param  {function} [errorCallback] function to be executed if
 *                               there is an error, response is passed
 *                               in as first argument
 * @return {String[]}            Array of Strings
 * @example
 *
 * <p>Calling loadStrings() inside preload() guarantees to complete the
 * operation before setup() and draw() are called.</p>
 *
 * <div><code>
 * var result;
 * function preload() {
 *   result = loadStrings('assets/test.txt');
 * }

 * function setup() {
 *   background(200);
 *   var ind = floor(random(result.length));
 *   text(result[ind], 10, 10, 80, 80);
 * }
 * </code></div>
 *
 * <p>Outside of preload(), you may supply a callback function to handle the
 * object:</p>
 *
 * <div><code>
 * function setup() {
 *   loadStrings('assets/test.txt', pickString);
 * }
 *
 * function pickString(result) {
 *   background(200);
 *   var ind = floor(random(result.length));
 *   text(result[ind], 10, 10, 80, 80);
 * }
 * </code></div>
 *
 * @alt
 * randomly generated text from a file, for example "i smell like butter"
 * randomly generated text from a file, for example "i have three feet"
 *
 */
p5.prototype.loadStrings = function () {
  var ret = [];
  var callback, errorCallback;

  for(var i=1; i<arguments.length; i++){
    var arg = arguments[i];
    if(typeof arg === 'function'){
      if(typeof callback === 'undefined'){
        callback = arg;
      }else if(typeof errorCallback === 'undefined'){
        errorCallback = arg;
      }
    }
  }

  var self = this;
  p5.prototype.httpDo.call(this, arguments[0], 'GET', 'text', function(data){
    var arr = data.match(/[^\r\n]+/g);
    for (var k in arr) {
      ret[k] = arr[k];
    }

    if (typeof callback !== 'undefined') {
      callback(ret);
    }

    self._decrementPreload();
  }, errorCallback);

  return ret;
};

/**
 * Method for executing an HTTP GET request. If data type is not specified,
 * p5 will try to guess based on the URL, defaulting to text. This is equivalent to
 * calling <code>httpDo(path, 'GET')</code>.
 *
 * @method httpGet
 * @param  {String}        path       name of the file or url to load
 * @param  {String}        [datatype] "json", "jsonp", "xml", or "text"
 * @param  {Object}        [data]     param data passed sent with request
 * @param  {function}      [callback] function to be executed after
 *                                    httpGet() completes, data is passed in
 *                                    as first argument
 * @param  {function}      [errorCallback] function to be executed if
 *                                    there is an error, response is passed
 *                                    in as first argument
 * @example
 * <div class='norender'><code>
*  // Examples use USGS Earthquake API:
*  //   https://earthquake.usgs.gov/fdsnws/event/1/#methods
*  var earthquakes;
*  function preload() {
*    // Get the most recent earthquake in the database
*    var url = 'https://earthquake.usgs.gov/fdsnws/event/1/query?' +
*      'format=geojson&limit=1&orderby=time';
*    httpGet(url, "jsonp", false, function(response) {
*      // when the HTTP request completes, populate the variable that holds the
*      // earthquake data used in the visualization.
*      earthquakes = response;
*    });
*  }
*
*  function draw() {
*    if (!earthquakes) {
*      // Wait until the earthquake data has loaded before drawing.
*      return
*    }
*    background(200);
*    // Get the magnitude and name of the earthquake out of the loaded JSON
*    var earthquakeMag = earthquakes.features[0].properties.mag;
*    var earthquakeName = earthquakes.features[0].properties.place;
*    ellipse(width/2, height/2, earthquakeMag * 10, earthquakeMag * 10);
*    textAlign(CENTER);
*    text(earthquakeName, 0, height - 30, width, 30);
*    noLoop();
*  }
 * </code></div>
 */
p5.prototype.httpGet = function () {
  var args = Array.prototype.slice.call(arguments);
  args.splice(1, 0, 'GET');
  p5.prototype.httpDo.apply(this, args);
};

/**
 * Method for executing an HTTP POST request. If data type is not specified,
 * p5 will try to guess based on the URL, defaulting to text. This is equivalent to
 * calling <code>httpDo(path, 'POST')</code>.
 *
 * @method httpPost
 * @param  {String}        path       name of the file or url to load
 * @param  {String}        [datatype] "json", "jsonp", "xml", or "text"
 * @param  {Object}        [data]     param data passed sent with request
 * @param  {function}      [callback] function to be executed after
 *                                    httpGet() completes, data is passed in
 *                                    as first argument
 * @param  {function}      [errorCallback] function to be executed if
 *                                    there is an error, response is passed
 *                                    in as first argument
 */
p5.prototype.httpPost = function () {
  var args = Array.prototype.slice.call(arguments);
  args.splice(1, 0, 'POST');
  p5.prototype.httpDo.apply(this, args);
};

/**
 * Method for executing an HTTP request. If data type is not specified,
 * p5 will try to guess based on the URL, defaulting to text.<br><br>
 * For more advanced use, you may also pass in the path as the first argument
 * and a object as the second argument, the signature follows the one specified
 * in the Fetch API specification.
 *
 * @method httpDo
 * @param  {String}        path       name of the file or url to load
 * @param  {String}        [method]   either "GET", "POST", or "PUT",
 *                                    defaults to "GET"
 * @param  {String}        [datatype] "json", "jsonp", "xml", or "text"
 * @param  {Object}        [data]     param data passed sent with request
 * @param  {function}      [callback] function to be executed after
 *                                    httpGet() completes, data is passed in
 *                                    as first argument
 * @param  {function}      [errorCallback] function to be executed if
 *                                    there is an error, response is passed
 *                                    in as first argument
 */

/**
 * @method httpDo
 * @param  {String}        path
 * @param  {Object}        options   Request object options as documented in the
 *                                    "fetch" API
 * <a href="https://developer.mozilla.org/en/docs/Web/API/Fetch_API">reference</a>
 * @param  {function}      [callback]
 * @param  {function}      [errorCallback]
 */
p5.prototype.httpDo = function () {
  var type = '';
  var callback;
  var errorCallback;
  var request;
  var jsonpOptions = {};
  var cbCount = 0;
  var contentType = 'text/plain';
  // Trim the callbacks off the end to get an idea of how many arguments are passed
  for (var i = arguments.length-1; i > 0; i--){
    if(typeof arguments[i] === 'function'){
      cbCount++;
    }else{
      break;
    }
  }
  // The number of arguments minus callbacks
  var argsCount = arguments.length - cbCount;
  if(argsCount === 2 &&
     typeof arguments[0] === 'string' &&
     typeof arguments[1] === 'object'){
    // Intended for more advanced use, pass in Request parameters directly
    request = new Request(arguments[0], arguments[1]);
    callback = arguments[2];
    errorCallback = arguments[3];

    // do some sort of smart type checking
    if (type === '') {
      if (request.url.indexOf('json') !== -1) {
        type = 'json';
      } else if (request.url.indexOf('xml') !== -1) {
        type = 'xml';
      } else {
        type = 'text';
      }
    }
  } else {
    // Provided with arguments
    var path = arguments[0];
    var method = 'GET';
    var data;

    for (var j = 1; j < arguments.length; j++) {
      var a = arguments[j];
      if (typeof a === 'string') {
        if (a === 'GET' || a === 'POST' || a === 'PUT' || a === 'DELETE') {
          method = a;
        } else if(a === 'json' || a === 'jsonp' || a === 'xml' || a === 'text') {
          type = a;
        } else {
          data = a;
        }
      } else if (typeof a === 'number') {
        data = a.toString();
      } else if (typeof a === 'object') {
        if(a.hasOwnProperty('jsonpCallback')){
          for (var attr in a) {
            jsonpOptions[attr] = a[attr];
          }
        }else{
          data = JSON.stringify(a);
          contentType = 'application/json';
        }
      } else if (typeof a === 'function') {
        if (!callback) {
          callback = a;
        } else {
          errorCallback = a;
        }
      }
    }
    // do some sort of smart type checking
    if (type === '') {
      if (path.indexOf('json') !== -1) {
        type = 'json';
      } else if (path.indexOf('xml') !== -1) {
        type = 'xml';
      } else {
        type = 'text';
      }
    }

    request = new Request(path, {
      method: method,
      mode: 'cors',
      body: data,
      headers: new Headers({
        'Content-Type': contentType
      })
    });
  }

  if(type === 'jsonp'){
    fetchJsonp(arguments[0], jsonpOptions)
      .then(function(res){
        if(res.ok){
          return res.json();
        }
        throw res;
      }).then(function(resp){
        callback(resp);
      }).catch(function(err){
        if (errorCallback) {
          errorCallback(err);
        } else {
          throw err;
        }
      });
  }else{
    fetch(request)
      .then(function(res){
        if(res.ok){
          if(type === 'json'){
            return res.json();
          }else{
            return res.text();
          }
        }

        throw res;
      })
      .then(function(resp){
        if (type === 'xml'){
          var parser = new DOMParser();
          resp = parser.parseFromString(resp, 'text/xml');
          resp = parseXML(resp.documentElement);
        }
        callback(resp);
      }).catch(function(err, msg){
        if (errorCallback) {
          errorCallback(err);
        } else {
          throw err;
        }
      });
  }
};

// =======
// HELPERS
// =======

/**
 *  Forces download. Accepts a url to filedata/blob, a filename,
 *  and an extension (optional).
 *  This is a private function because it does not do any formatting,
 *  but it is used by saveStrings, saveJSON, saveTable etc.
 *
 *  @param  {String} href      i.e. an href generated by createObjectURL
 *  @param  {[String]} filename
 *  @param  {[String]} extension
 */
p5.prototype.downloadFile = function (href, fName, extension) {
  var fx = _checkFileExtension(fName, extension);
  var filename = fx[0];
  var ext = fx[1];

  var a = document.createElement('a');
  a.href = href;
  a.download = filename;

  // Firefox requires the link to be added to the DOM before click()
  a.onclick = function(e) {
    destroyClickedElement(e);
    e.stopPropagation();
  };

  a.style.display = 'none';
  document.body.appendChild(a);

  // Safari will open this file in the same page as a confusing Blob.
  if (p5.prototype._isSafari()) {
    var aText = 'Hello, Safari user! To download this file...\n';
    aText += '1. Go to File --> Save As.\n';
    aText += '2. Choose "Page Source" as the Format.\n';
    aText += '3. Name it with this extension: .\"' + ext + '\"';
    alert(aText);
  }
  a.click();
  href = null;
};

/**
 *  Returns a file extension, or another string
 *  if the provided parameter has no extension.
 *
 *  @param   {String} filename
 *  @return  {String[]} [fileName, fileExtension]
 *
 *  @private
 */
function _checkFileExtension(filename, extension) {
  if (!extension || extension === true || extension === 'true') {
    extension = '';
  }
  if (!filename) {
    filename = 'untitled';
  }
  var ext = '';
  // make sure the file will have a name, see if filename needs extension
  if (filename && filename.indexOf('.') > -1) {
    ext = filename.split('.').pop();
  }
  // append extension if it doesn't exist
  if (extension) {
    if (ext !== extension) {
      ext = extension;
      filename = filename + '.' + ext;
    }
  }
  return [filename, ext];
}
p5.prototype._checkFileExtension = _checkFileExtension;

/**
 *  Returns true if the browser is Safari, false if not.
 *  Safari makes trouble for downloading files.
 *
 *  @return  {Boolean} [description]
 *  @private
 */
p5.prototype._isSafari = function () {
  var x = Object.prototype.toString.call(window.HTMLElement);
  return x.indexOf('Constructor') > 0;
};

/**
 *  Helper function, a callback for download that deletes
 *  an invisible anchor element from the DOM once the file
 *  has been automatically downloaded.
 *
 *  @private
 */
function destroyClickedElement(event) {
  document.body.removeChild(event.target);
}

module.exports = p5;

},{"../core/core":54,"../core/error_helpers":57}],82:[function(_dereq_,module,exports){
/**
 * @module Math
 * @submodule Calculation
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');

/**
 * Calculates the absolute value (magnitude) of a number. Maps to Math.abs().
 * The absolute value of a number is always positive.
 *
 * @method abs
 * @param  {Number} n number to compute
 * @return {Number}   absolute value of given number
 * @example
 * <div class = "norender"><code>
 * function setup() {
 *   var x = -3;
 *   var y = abs(x);
 *
 *   print(x); // -3
 *   print(y); // 3
 * }
 * </code></div>
 *
 * @alt
 * no image displayed
 *
 */
p5.prototype.abs = Math.abs;

/**
 * Calculates the closest int value that is greater than or equal to the
 * value of the parameter. Maps to Math.ceil(). For example, ceil(9.03)
 * returns the value 10.
 *
 * @method ceil
 * @param  {Number} n number to round up
 * @return {Number}   rounded up number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   // map, mouseX between 0 and 5.
 *   var ax = map(mouseX, 0, 100, 0, 5);
 *   var ay = 66;
 *
 *   //Get the ceiling of the mapped number.
 *   var bx = ceil(map(mouseX, 0, 100, 0,5));
 *   var by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2,2), ax, ay - 5);
 *   text(nfc(bx,1,1), bx, by - 5);
 * }
 * </code></div>
  *
 * @alt
 * 2 horizontal lines & number sets. increase with mouse x. bottom to 2 decimals
 *
 */
p5.prototype.ceil = Math.ceil;

/**
 * Constrains a value between a minimum and maximum value.
 *
 * @method constrain
 * @param  {Number} n    number to constrain
 * @param  {Number} low  minimum limit
 * @param  {Number} high maximum limit
 * @return {Number}      constrained number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *
 *   var leftWall = 25;
 *   var rightWall = 75;
 *
 *   // xm is just the mouseX, while
 *   // xc is the mouseX, but constrained
 *   // between the leftWall and rightWall!
 *   var xm = mouseX;
 *   var xc = constrain(mouseX, leftWall, rightWall);
 *
 *   // Draw the walls.
 *   stroke(150);
 *   line(leftWall, 0, leftWall, height);
 *   line(rightWall, 0, rightWall, height);
 *
 *   // Draw xm and xc as circles.
 *   noStroke();
 *   fill(150);
 *   ellipse(xm, 33, 9,9); // Not Constrained
 *   fill(0);
 *   ellipse(xc, 66, 9,9); // Constrained
 * }
 * </code></div>
 *
 * @alt
 * 2 vertical lines. 2 ellipses move with mouse X 1 does not move passed lines
 *
 */
p5.prototype.constrain = function(n, low, high) {
  return Math.max(Math.min(n, high), low);
};

/**
 * Calculates the distance between two points.
 *
 * @method dist
 * @param  {Number} x1 x-coordinate of the first point
 * @param  {Number} y1 y-coordinate of the first point
 * @param  {Number} x2 x-coordinate of the second point
 * @param  {Number} y2 y-coordinate of the second point
 * @return {Number}    distance between the two points
 */
/**
 * @method dist
 * @param  {Number} x1
 * @param  {Number} y1
 * @param  {Number} z1 z-coordinate of the first point
 * @param  {Number} x2
 * @param  {Number} y2
 * @param  {Number} z2 z-coordinate of the second point
 * @return {Number}    distance between the two points
 * @example
 * <div><code>
 * // Move your mouse inside the canvas to see the
 * // change in distance between two points!
 * function draw() {
 *   background(200);
 *   fill(0);
 *
 *   var x1 = 10;
 *   var y1 = 90;
 *   var x2 = mouseX;
 *   var y2 = mouseY;
 *
 *   line(x1, y1, x2, y2);
 *   ellipse(x1, y1, 7, 7);
 *   ellipse(x2, y2, 7, 7);
 *
 *   // d is the length of the line
 *   // the distance from point 1 to point 2.
 *   var d = int(dist(x1, y1, x2, y2));
 *
 *   // Let's write d along the line we are drawing!
 *   push();
 *   translate( (x1+x2)/2, (y1+y2)/2 );
 *   rotate( atan2(y2-y1,x2-x1) );
 *   text(nfc(d,1,1), 0, -5);
 *   pop();
 *   // Fancy!
 * }
 * </code></div>
 *
 * @alt
 * 2 ellipses joined by line. 1 ellipse moves with mouse X&Y. Distance displayed.
 *
 */
p5.prototype.dist = function(x1, y1, z1, x2, y2, z2) {
  if (arguments.length === 4) {
    // In the case of 2d: z1 means x2 and x2 means y2
    return hypot(z1-x1, x2-y1);
  } else if (arguments.length === 6) {
    return hypot(x2-x1, y2-y1, z2-z1);
  }
};

/**
 * Returns Euler's number e (2.71828...) raised to the power of the n
 * parameter. Maps to Math.exp().
 *
 * @method exp
 * @param  {Number} n exponent to raise
 * @return {Number}   e^n
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *
 *   // Compute the exp() function with a value between 0 and 2
 *   var xValue = map(mouseX, 0, width, 0, 2);
 *   var yValue = exp(xValue);
 *
 *   var y = map(yValue, 0, 8, height, 0);
 *
 *   var legend = "exp (" + nfc(xValue, 3) +")\n= " + nf(yValue, 1, 4);
 *   stroke(150);
 *   line(mouseX, y, mouseX, height);
 *   fill(0);
 *   text(legend, 5, 15);
 *   noStroke();
 *   ellipse (mouseX,y, 7, 7);
 *
 *   // Draw the exp(x) curve,
 *   // over the domain of x from 0 to 2
 *   noFill();
 *   stroke(0);
 *   beginShape();
 *   for (var x = 0; x < width; x++) {
 *     xValue = map(x, 0, width, 0, 2);
 *     yValue = exp(xValue);
 *     y = map(yValue, 0, 8, height, 0);
 *     vertex(x, y);
 *   }
 *
 *   endShape();
 *   line(0, 0, 0, height);
 *   line(0, height-1, width, height-1);
 * }
 * </code></div>
 *
 * @alt
 * ellipse moves along a curve with mouse x. e^n displayed.
 *
 */
p5.prototype.exp = Math.exp;

/**
 * Calculates the closest int value that is less than or equal to the
 * value of the parameter. Maps to Math.floor().
 *
 * @method floor
 * @param  {Number} n number to round down
 * @return {Number}   rounded down number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   //map, mouseX between 0 and 5.
 *   var ax = map(mouseX, 0, 100, 0, 5);
 *   var ay = 66;
 *
 *   //Get the floor of the mapped number.
 *   var bx = floor(map(mouseX, 0, 100, 0,5));
 *   var by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2,2), ax, ay - 5);
 *   text(nfc(bx,1,1), bx, by - 5);
 * }
 * </code></div>
 *
 * @alt
 * 2 horizontal lines & number sets. increase with mouse x. bottom to 2 decimals
 *
 */
p5.prototype.floor = Math.floor;

/**
 * Calculates a number between two numbers at a specific increment. The amt
 * parameter is the amount to interpolate between the two values where 0.0
 * equal to the first point, 0.1 is very near the first point, 0.5 is
 * half-way in between, etc. The lerp function is convenient for creating
 * motion along a straight path and for drawing dotted lines.
 *
 * @method lerp
 * @param  {Number} start first value
 * @param  {Number} stop  second value
 * @param  {Number} amt   number between 0.0 and 1.0
 * @return {Number}       lerped value
 * @example
 * <div><code>
 * function setup() {
 *   background(200);
 *   var a = 20;
 *   var b = 80;
 *   var c = lerp(a,b, .2);
 *   var d = lerp(a,b, .5);
 *   var e = lerp(a,b, .8);
 *
 *   var y = 50
 *
 *   strokeWeight(5);
 *   stroke(0); // Draw the original points in black
 *   point(a, y);
 *   point(b, y);
 *
 *   stroke(100); // Draw the lerp points in gray
 *   point(c, y);
 *   point(d, y);
 *   point(e, y);
 * }
 * </code></div>
 *
 * @alt
 * 5 points horizontally staggered mid-canvas. mid 3 are grey, outer black
 *
 */
p5.prototype.lerp = function(start, stop, amt) {
  return amt*(stop-start)+start;
};

/**
 * Calculates the natural logarithm (the base-e logarithm) of a number. This
 * function expects the n parameter to be a value greater than 0.0. Maps to
 * Math.log().
 *
 * @method log
 * @param  {Number} n number greater than 0
 * @return {Number}   natural logarithm of n
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   var maxX = 2.8;
 *   var maxY = 1.5;
 *
 *   // Compute the natural log of a value between 0 and maxX
 *   var xValue = map(mouseX, 0, width, 0, maxX);
 *   if (xValue > 0) { // Cannot take the log of a negative number.
 *     var yValue = log(xValue);
 *     var y = map(yValue, -maxY, maxY, height, 0);
 *
 *     // Display the calculation occurring.
 *     var legend = "log(" + nf(xValue, 1, 2) + ")\n= " + nf(yValue, 1, 3);
 *     stroke(150);
 *     line(mouseX, y, mouseX, height);
 *     fill(0);
 *     text (legend, 5, 15);
 *     noStroke();
 *     ellipse (mouseX, y, 7, 7);
 *   }
 *
 *   // Draw the log(x) curve,
 *   // over the domain of x from 0 to maxX
 *   noFill();
 *   stroke(0);
 *   beginShape();
 *   for(var x=0; x < width; x++) {
 *     xValue = map(x, 0, width, 0, maxX);
 *     yValue = log(xValue);
 *     y = map(yValue, -maxY, maxY, height, 0);
 *     vertex(x, y);
 *   }
 *   endShape();
 *   line(0,0,0,height);
 *   line(0,height/2,width, height/2);
 * }
 * </code></div>
 *
 * @alt
 * ellipse moves along a curve with mouse x. natural logarithm of n displayed.
 *
 */
p5.prototype.log = Math.log;

/**
 * Calculates the magnitude (or length) of a vector. A vector is a direction
 * in space commonly used in computer graphics and linear algebra. Because it
 * has no "start" position, the magnitude of a vector can be thought of as
 * the distance from the coordinate 0,0 to its x,y value. Therefore, mag() is
 * a shortcut for writing dist(0, 0, x, y).
 *
 * @method mag
 * @param  {Number} a first value
 * @param  {Number} b second value
 * @return {Number}   magnitude of vector from (0,0) to (a,b)
 * @example
 * <div><code>
 * function setup() {
 *   var x1 = 20;
 *   var x2 = 80;
 *   var y1 = 30;
 *   var y2 = 70;
 *
 *   line(0, 0, x1, y1);
 *   print(mag(x1, y1));  // Prints "36.05551275463989"
 *   line(0, 0, x2, y1);
 *   print(mag(x2, y1));  // Prints "85.44003745317531"
 *   line(0, 0, x1, y2);
 *   print(mag(x1, y2));  // Prints "72.80109889280519"
 *   line(0, 0, x2, y2);
 *   print(mag(x2, y2));  // Prints "106.3014581273465"
 * }
 * </code></div>
 *
 * @alt
 * 4 lines of different length radiate from top left of canvas.
 *
 */
p5.prototype.mag = function(x, y) {
  return hypot(x, y);
};

/**
 * Re-maps a number from one range to another.
 * <br><br>
 * In the first example above, the number 25 is converted from a value in the
 * range of 0 to 100 into a value that ranges from the left edge of the
 * window (0) to the right edge (width).
 *
 * @method map
 * @param  {Number} value  the incoming value to be converted
 * @param  {Number} start1 lower bound of the value's current range
 * @param  {Number} stop1  upper bound of the value's current range
 * @param  {Number} start2 lower bound of the value's target range
 * @param  {Number} stop2  upper bound of the value's target range
 * @return {Number}        remapped number
 * @example
 *   <div><code>
 *     var value = 25;
 *     var m = map(value, 0, 100, 0, width);
 *     ellipse(m, 50, 10, 10);
 *   </code></div>
 *
 *   <div><code>
 *     function setup() {
 *       noStroke();
 *     }
 *
 *     function draw() {
 *       background(204);
 *       var x1 = map(mouseX, 0, width, 25, 75);
 *       ellipse(x1, 25, 25, 25);
 *       var x2 = map(mouseX, 0, width, 0, 100);
 *       ellipse(x2, 75, 25, 25);
 *     }
 *   </code></div>
 *
 * @alt
 * 10 by 10 white ellipse with in mid left canvas
 * 2 25 by 25 white ellipses move with mouse x. Bottom has more range from X
 *
 */
p5.prototype.map = function(n, start1, stop1, start2, stop2) {
  return ((n-start1)/(stop1-start1))*(stop2-start2)+start2;
};

/**
 * Determines the largest value in a sequence of numbers, and then returns
 * that value. max() accepts any number of Number parameters, or an Array
 * of any length.
 *
 * @method max
 * @param  {Number|Array} n0 Numbers to compare
 * @return {Number}          maximum Number
 * @example
 * <div><code>
 * function setup() {
 *   // Change the elements in the array and run the sketch
 *   // to show how max() works!
 *   numArray = new Array(2,1,5,4,8,9);
 *   fill(0);
 *   noStroke();
 *   text("Array Elements", 0, 10);
 *   // Draw all numbers in the array
 *   var spacing = 15;
 *   var elemsY = 25;
 *   for(var i = 0; i < numArray.length; i++) {
 *     text(numArray[i], i * spacing, elemsY);
 *   }
 *   maxX = 33;
 *   maxY = 80;
 *   // Draw the Maximum value in the array.
 *   textSize(32);
 *   text(max(numArray), maxX, maxY);
 * }
 * </code></div>
 *
 * @alt
 * Small text at top reads: Array Elements 2 1 5 4 8 9. Large text at center: 9
 *
 */
p5.prototype.max = function() {
  if (arguments[0] instanceof Array) {
    return Math.max.apply(null,arguments[0]);
  } else {
    return Math.max.apply(null,arguments);
  }
};

/**
 * Determines the smallest value in a sequence of numbers, and then returns
 * that value. min() accepts any number of Number parameters, or an Array
 * of any length.
 *
 * @method min
 * @param  {Number|Array} n0 Numbers to compare
 * @return {Number}          minimum Number
 * @example
 * <div><code>
 * function setup() {
 *   // Change the elements in the array and run the sketch
 *   // to show how min() works!
 *   numArray = new Array(2,1,5,4,8,9);
 *   fill(0);
 *   noStroke();
 *   text("Array Elements", 0, 10);
 *   // Draw all numbers in the array
 *   var spacing = 15;
 *   var elemsY = 25;
 *   for(var i = 0; i < numArray.length; i++) {
 *     text(numArray[i], i * spacing, elemsY);
 *   }
 *   maxX = 33;
 *   maxY = 80;
 *   // Draw the Minimum value in the array.
 *   textSize(32);
 *   text(min(numArray), maxX, maxY);
 * }
 * </code></div>
 *
 * @alt
 * Small text at top reads: Array Elements 2 1 5 4 8 9. Large text at center: 1
 *
 */
p5.prototype.min = function() {
  if (arguments[0] instanceof Array) {
    return Math.min.apply(null,arguments[0]);
  } else {
    return Math.min.apply(null,arguments);
  }
};

/**
 * Normalizes a number from another range into a value between 0 and 1.
 * Identical to map(value, low, high, 0, 1).
 * Numbers outside of the range are not clamped to 0 and 1, because
 * out-of-range values are often intentional and useful. (See the second
 * example above.)
 *
 * @method norm
 * @param  {Number} value incoming value to be normalized
 * @param  {Number} start lower bound of the value's current range
 * @param  {Number} stop  upper bound of the value's current range
 * @return {Number}       normalized number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   currentNum = mouseX;
 *   lowerBound = 0;
 *   upperBound = width; //100;
 *   normalized = norm(currentNum, lowerBound, upperBound);
 *   lineY = 70
 *   line(0, lineY, width, lineY);
 *   //Draw an ellipse mapped to the non-normalized value.
 *   noStroke();
 *   fill(50)
 *   var s = 7; // ellipse size
 *   ellipse(currentNum, lineY, s, s);
 *
 *   // Draw the guide
 *   guideY = lineY + 15;
 *   text("0", 0, guideY);
 *   textAlign(RIGHT);
 *   text("100", width, guideY);
 *
 *   // Draw the normalized value
 *   textAlign(LEFT);
 *   fill(0);
 *   textSize(32);
 *   normalY = 40;
 *   normalX = 20;
 *   text(normalized, normalX, normalY);
 * }
 * </code></div>
 *
 * @alt
 * ellipse moves with mouse. 0 shown left & 100 right and updating values center
 *
 */
p5.prototype.norm = function(n, start, stop) {
  return this.map(n, start, stop, 0, 1);
};

/**
 * Facilitates exponential expressions. The pow() function is an efficient
 * way of multiplying numbers by themselves (or their reciprocals) in large
 * quantities. For example, pow(3, 5) is equivalent to the expression
 * 3*3*3*3*3 and pow(3, -5) is equivalent to 1 / 3*3*3*3*3. Maps to
 * Math.pow().
 *
 * @method pow
 * @param  {Number} n base of the exponential expression
 * @param  {Number} e power by which to raise the base
 * @return {Number}   n^e
 * @example
 * <div><code>
 * function setup() {
 *   //Exponentially increase the size of an ellipse.
 *   eSize = 3; // Original Size
 *   eLoc = 10; // Original Location
 *
 *   ellipse(eLoc, eLoc, eSize, eSize);
 *
 *   ellipse(eLoc*2, eLoc*2, pow(eSize, 2), pow(eSize, 2));
 *
 *   ellipse(eLoc*4, eLoc*4, pow(eSize, 3), pow(eSize, 3));
 *
 *   ellipse(eLoc*8, eLoc*8, pow(eSize, 4), pow(eSize, 4));
 * }
 * </code></div>
 *
 * @alt
 * small to large ellipses radiating from top left of canvas
 *
 */
p5.prototype.pow = Math.pow;

/**
 * Calculates the integer closest to the n parameter. For example,
 * round(133.8) returns the value 134. Maps to Math.round().
 *
 * @method round
 * @param  {Number} n number to round
 * @return {Number}   rounded number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   //map, mouseX between 0 and 5.
 *   var ax = map(mouseX, 0, 100, 0, 5);
 *   var ay = 66;
 *
 *   // Round the mapped number.
 *   var bx = round(map(mouseX, 0, 100, 0,5));
 *   var by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2,2), ax, ay - 5);
 *   text(nfc(bx,1,1), bx, by - 5);
 * }
 * </code></div>
 *
 * @alt
 * horizontal center line squared values displayed on top and regular on bottom.
 *
 */
p5.prototype.round = Math.round;

/**
 * Squares a number (multiplies a number by itself). The result is always a
 * positive number, as multiplying two negative numbers always yields a
 * positive result. For example, -1 * -1 = 1.
 *
 * @method sq
 * @param  {Number} n number to square
 * @return {Number}   squared number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   eSize = 7;
 *   x1 = map(mouseX, 0, width, 0, 10);
 *   y1 = 80;
 *   x2 = sq(x1);
 *   y2 = 20;
 *
 *   // Draw the non-squared.
 *   line(0, y1, width, y1);
 *   ellipse(x1, y1, eSize, eSize);
 *
 *   // Draw the squared.
 *   line(0, y2, width, y2);
 *   ellipse(x2, y2, eSize, eSize);
 *
 *   // Draw dividing line.
 *   stroke(100)
 *   line(0, height/2, width, height/2);
 *
 *   // Draw text.
 *   var spacing = 15;
 *   noStroke();
 *   fill(0);
 *   text("x = " + x1, 0, y1 + spacing);
 *   text("sq(x) = " + x2, 0, y2 + spacing);
 * }
 * </code></div>
 *
 * @alt
 * horizontal center line squared values displayed on top and regular on bottom.
 *
 */
p5.prototype.sq = function(n) { return n*n; };

/**
 * Calculates the square root of a number. The square root of a number is
 * always positive, even though there may be a valid negative root. The
 * square root s of number a is such that s*s = a. It is the opposite of
 * squaring. Maps to Math.sqrt().
 *
 * @method sqrt
 * @param  {Number} n non-negative number to square root
 * @return {Number}   square root of number
 * @example
 * <div><code>
 * function draw() {
 *   background(200);
 *   eSize = 7;
 *   x1 = mouseX;
 *   y1 = 80;
 *   x2 = sqrt(x1);
 *   y2 = 20;
 *
 *   // Draw the non-squared.
 *   line(0, y1, width, y1);
 *   ellipse(x1, y1, eSize, eSize);
 *
 *   // Draw the squared.
 *   line(0, y2, width, y2);
 *   ellipse(x2, y2, eSize, eSize);
 *
 *   // Draw dividing line.
 *   stroke(100)
 *   line(0, height/2, width, height/2);
 *
 *   // Draw text.
 *   noStroke();
 *   fill(0);
 *   var spacing = 15;
 *   text("x = " + x1, 0, y1 + spacing);
 *   text("sqrt(x) = " + x2, 0, y2 + spacing);
 * }
 * </code></div>
 *
 * @alt
 * horizontal center line squareroot values displayed on top and regular on bottom.
 *
 */
p5.prototype.sqrt = Math.sqrt;

// Calculate the length of the hypotenuse of a right triangle
// This won't under- or overflow in intermediate steps
// https://en.wikipedia.org/wiki/Hypot
function hypot(x, y, z) {
  // Use the native implementation if it's available
  if (typeof Math.hypot === 'function') {
    return Math.hypot.apply(null, arguments);
  }

  // Otherwise use the V8 implementation
  // https://github.com/v8/v8/blob/8cd3cf297287e581a49e487067f5cbd991b27123/src/js/math.js#L217
  var length = arguments.length;
  var args = [];
  var max = 0;
  for (var i = 0; i < length; i++) {
    var n = arguments[i];
    n = +n;
    if (n === Infinity || n === -Infinity) {
      return Infinity;
    }
    n = Math.abs(n);
    if (n > max) {
      max = n;
    }
    args[i] = n;
  }

  if (max === 0) {
    max = 1;
  }
  var sum = 0;
  var compensation = 0;
  for (var j = 0; j < length; j++) {
    var m = args[j] / max;
    var summand = m * m - compensation;
    var preliminary = sum + summand;
    compensation = (preliminary - sum) - summand;
    sum = preliminary;
  }
  return Math.sqrt(sum) * max;
}

module.exports = p5;

},{"../core/core":54}],83:[function(_dereq_,module,exports){
/**
 * @module Math
 * @submodule Math
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');


/**
 * Creates a new p5.Vector (the datatype for storing vectors). This provides a
 * two or three dimensional vector, specifically a Euclidean (also known as
 * geometric) vector. A vector is an entity that has both magnitude and
 * direction.
 *
 * @method createVector
 * @param {Number} [x] x component of the vector
 * @param {Number} [y] y component of the vector
 * @param {Number} [z] z component of the vector
 * @return {p5.Vector}
 */
p5.prototype.createVector = function (x, y, z) {
  if (this instanceof p5) {
    return new p5.Vector(this, arguments);
  } else {
    return new p5.Vector(x, y, z);
  }
};

module.exports = p5;

},{"../core/core":54}],85:[function(_dereq_,module,exports){
/**
 * @module Math
 * @submodule Math
 * @requires constants
 */

'use strict';

var p5 = _dereq_('../core/core');
var polarGeometry = _dereq_('./polargeometry');
var constants = _dereq_('../core/constants');

/**
 * A class to describe a two or three dimensional vector, specifically
 * a Euclidean (also known as geometric) vector. A vector is an entity
 * that has both magnitude and direction. The datatype, however, stores
 * the components of the vector (x, y for 2D, and x, y, z for 3D). The magnitude
 * and direction can be accessed via the methods mag() and heading().
 * <br><br>
 * In many of the p5.js examples, you will see p5.Vector used to describe a
 * position, velocity, or acceleration. For example, if you consider a rectangle
 * moving across the screen, at any given instant it has a position (a vector
 * that points from the origin to its location), a velocity (the rate at which
 * the object's position changes per time unit, expressed as a vector), and
 * acceleration (the rate at which the object's velocity changes per time
 * unit, expressed as a vector).
 * <br><br>
 * Since vectors represent groupings of values, we cannot simply use
 * traditional addition/multiplication/etc. Instead, we'll need to do some
 * "vector" math, which is made easy by the methods inside the p5.Vector class.
 *
 * @class p5.Vector
 * @constructor
 * @param {Number} [x] x component of the vector
 * @param {Number} [y] y component of the vector
 * @param {Number} [z] z component of the vector
 * @example
 * <div>
 * <code>
 * var v1 = createVector(40, 50);
 * var v2 = createVector(40, 50);
 *
 * ellipse(v1.x, v1.y, 50, 50);
 * ellipse(v2.x, v2.y, 50, 50);
 * v1.add(v2);
 * ellipse(v1.x, v1.y, 50, 50);
 * </code>
 * </div>
 *
 * @alt
 * 2 white ellipses. One center-left the other bottom right and off canvas
 *
 */
p5.Vector = function() {
  var x,y,z;
  // This is how it comes in with createVector()
  if(arguments[0] instanceof p5) {
    // save reference to p5 if passed in
    this.p5 = arguments[0];
    x  = arguments[1][0] || 0;
    y  = arguments[1][1] || 0;
    z  = arguments[1][2] || 0;
  // This is what we'll get with new p5.Vector()
  } else {
    x = arguments[0] || 0;
    y = arguments[1] || 0;
    z = arguments[2] || 0;
  }
  /**
   * The x component of the vector
   * @property x {Number}
   */
  this.x = x;
  /**
   * The y component of the vector
   * @property y {Number}
   */
  this.y = y;
  /**
   * The z component of the vector
   * @property z {Number}
   */
  this.z = z;
};

/**
 * Returns a string representation of a vector v by calling String(v)
 * or v.toString(). This method is useful for logging vectors in the
 * console.
 * @method  toString
 * @example
 * <div class = "norender"><code>
 * function setup() {
 *   var v = createVector(20,30);
 *   print(String(v)); // prints "p5.Vector Object : [20, 30, 0]"
 * }
 * </div></code>
 *
 */
p5.Vector.prototype.toString = function p5VectorToString() {
  return 'p5.Vector Object : ['+ this.x +', '+ this.y +', '+ this.z + ']';
};

/**
 * Sets the x, y, and z component of the vector using two or three separate
 * variables, the data from a p5.Vector, or the values from a float array.
 * @method set
 * @param {Number|p5.Vector|Array} [x] the x component of the vector or a
 *                                     p5.Vector or an Array
 * @param {Number}                 [y] the y component of the vector
 * @param {Number}                 [z] the z component of the vector
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * function setup() {
 *    var v = createVector(1, 2, 3);
 *    v.set(4,5,6); // Sets vector to [4, 5, 6]
 *
 *    var v1 = createVector(0, 0, 0);
 *    var arr = [1, 2, 3];
 *    v1.set(arr); // Sets vector to [1, 2, 3]
 * }
 * </code>
 * </div>
 */
p5.Vector.prototype.set = function (x, y, z) {
  if (x instanceof p5.Vector) {
    this.x = x.x || 0;
    this.y = x.y || 0;
    this.z = x.z || 0;
    return this;
  }
  if (x instanceof Array) {
    this.x = x[0] || 0;
    this.y = x[1] || 0;
    this.z = x[2] || 0;
    return this;
  }
  this.x = x || 0;
  this.y = y || 0;
  this.z = z || 0;
  return this;
};

/**
 * Gets a copy of the vector, returns a p5.Vector object.
 *
 * @method copy
 * @return {p5.Vector} the copy of the p5.Vector object
 * @example
 * <div class="norender">
 * <code>
 * var v1 = createVector(1, 2, 3);
 * var v2 = v1.copy();
 * print(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
 * // Prints "true"
 * </code>
 * </div>
 */
p5.Vector.prototype.copy = function () {
  if (this.p5) {
    return new p5.Vector(this.p5,[this.x, this.y, this.z]);
  } else {
    return new p5.Vector(this.x,this.y,this.z);
  }
};

/**
 * Adds x, y, and z components to a vector, adds one vector to another, or
 * adds two independent vectors together. The version of the method that adds
 * two vectors together is a static method and returns a p5.Vector, the others
 * acts directly on the vector. See the examples for more context.
 *
 * @method add
 * @param  {Number|p5.Vector|Array} x   the x component of the vector to be
 *                                      added or a p5.Vector or an Array
 * @param  {Number}                 [y] the y component of the vector to be
 *                                      added
 * @param  {Number}                 [z] the z component of the vector to be
 *                                      added
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(1, 2, 3);
 * v.add(4,5,6);
 * // v's components are set to [5, 7, 9]
 * </code>
 * </div>
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(1, 2, 3);
 * var v2 = createVector(2, 3, 4);
 *
 * var v3 = p5.Vector.add(v1, v2);
 * // v3 has components [3, 5, 7]
 * </code>
 * </div>
 */
p5.Vector.prototype.add = function (x, y, z) {
  if (x instanceof p5.Vector) {
    this.x += x.x || 0;
    this.y += x.y || 0;
    this.z += x.z || 0;
    return this;
  }
  if (x instanceof Array) {
    this.x += x[0] || 0;
    this.y += x[1] || 0;
    this.z += x[2] || 0;
    return this;
  }
  this.x += x || 0;
  this.y += y || 0;
  this.z += z || 0;
  return this;
};

/**
 * Subtracts x, y, and z components from a vector, subtracts one vector from
 * another, or subtracts two independent vectors. The version of the method
 * that subtracts two vectors is a static method and returns a p5.Vector, the
 * other acts directly on the vector. See the examples for more context.
 *
 * @method sub
 * @param  {Number|p5.Vector|Array} x   the x component of the vector or a
 *                                      p5.Vector or an Array
 * @param  {Number}                 [y] the y component of the vector
 * @param  {Number}                 [z] the z component of the vector
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(4, 5, 6);
 * v.sub(1, 1, 1);
 * // v's components are set to [3, 4, 5]
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(2, 3, 4);
 * var v2 = createVector(1, 2, 3);
 *
 * var v3 = p5.Vector.sub(v1, v2);
 * // v3 has components [1, 1, 1]
 * </code>
 * </div>
 */
p5.Vector.prototype.sub = function (x, y, z) {
  if (x instanceof p5.Vector) {
    this.x -= x.x || 0;
    this.y -= x.y || 0;
    this.z -= x.z || 0;
    return this;
  }
  if (x instanceof Array) {
    this.x -= x[0] || 0;
    this.y -= x[1] || 0;
    this.z -= x[2] || 0;
    return this;
  }
  this.x -= x || 0;
  this.y -= y || 0;
  this.z -= z || 0;
  return this;
};

/**
 * Multiply the vector by a scalar. The static version of this method
 * creates a new p5.Vector while the non static version acts on the vector
 * directly. See the examples for more context.
 *
 * @method mult
 * @param  {Number}    n the number to multiply with the vector
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(1, 2, 3);
 * v.mult(2);
 * // v's components are set to [2, 4, 6]
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(1, 2, 3);
 * var v2 = p5.Vector.mult(v1, 2);
 * // v2 has components [2, 4, 6]
 * </code>
 * </div>
 */
p5.Vector.prototype.mult = function (n) {
  this.x *= n || 0;
  this.y *= n || 0;
  this.z *= n || 0;
  return this;
};

/**
 * Divide the vector by a scalar. The static version of this method creates a
 * new p5.Vector while the non static version acts on the vector directly.
 * See the examples for more context.
 *
 * @method div
 * @param  {number}    n the number to divide the vector by
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(6, 4, 2);
 * v.div(2); //v's components are set to [3, 2, 1]
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * // Static method
 * var v1  = createVector(6, 4, 2);
 * var v2 = p5.Vector.div(v, 2);
 * // v2 has components [3, 2, 1]
 * </code>
 * </div>
 */
p5.Vector.prototype.div = function (n) {
  this.x /= n;
  this.y /= n;
  this.z /= n;
  return this;
};

/**
 * Calculates the magnitude (length) of the vector and returns the result as
 * a float (this is simply the equation sqrt(x*x + y*y + z*z).)
 *
 * @method mag
 * @return {Number} magnitude of the vector
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(20.0, 30.0, 40.0);
 * var m = v.mag();
 * print(m); // Prints "53.85164807134504"
 * </code>
 * </div>
 */
p5.Vector.prototype.mag = function () {
  return Math.sqrt(this.magSq());
};

/**
 * Calculates the squared magnitude of the vector and returns the result
 * as a float (this is simply the equation <em>(x*x + y*y + z*z)</em>.)
 * Faster if the real length is not required in the
 * case of comparing vectors, etc.
 *
 * @method magSq
 * @return {number} squared magnitude of the vector
 * @example
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(6, 4, 2);
 * print(v1.magSq()); // Prints "56"
 * </code>
 * </div>
 */
p5.Vector.prototype.magSq = function () {
  var x = this.x, y = this.y, z = this.z;
  return (x * x + y * y + z * z);
};

/**
 * Calculates the dot product of two vectors. The version of the method
 * that computes the dot product of two independent vectors is a static
 * method. See the examples for more context.
 *
 *
 * @method dot
 * @param  {Number|p5.Vector} x   x component of the vector or a p5.Vector
 * @param  {Number}           [y] y component of the vector
 * @param  {Number}           [z] z component of the vector
 * @return {Number}                 the dot product
 *
 * @example
 * <div class="norender">
 * <code>
 * var v1 = createVector(1, 2, 3);
 * var v2 = createVector(2, 3, 4);
 *
 * print(v1.dot(v2)); // Prints "20"
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * //Static method
 * var v1 = createVector(1, 2, 3);
 * var v2 = createVector(3, 2, 1);
 * print (p5.Vector.dot(v1, v2)); // Prints "10"
 * </code>
 * </div>
 */
p5.Vector.prototype.dot = function (x, y, z) {
  if (x instanceof p5.Vector) {
    return this.dot(x.x, x.y, x.z);
  }
  return this.x * (x || 0) +
         this.y * (y || 0) +
         this.z * (z || 0);
};

/**
 * Calculates and returns a vector composed of the cross product between
 * two vectors. Both the static and non static methods return a new p5.Vector.
 * See the examples for more context.
 *
 * @method cross
 * @param  {p5.Vector} v p5.Vector to be crossed
 * @return {p5.Vector}   p5.Vector composed of cross product
 * @example
 * <div class="norender">
 * <code>
 * var v1 = createVector(1, 2, 3);
 * var v2 = createVector(1, 2, 3);
 *
 * v1.cross(v2); // v's components are [0, 0, 0]
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(1, 0, 0);
 * var v2 = createVector(0, 1, 0);
 *
 * var crossProduct = p5.Vector.cross(v1, v2);
 * // crossProduct has components [0, 0, 1]
 * </code>
 * </div>
 */
p5.Vector.prototype.cross = function (v) {
  var x = this.y * v.z - this.z * v.y;
  var y = this.z * v.x - this.x * v.z;
  var z = this.x * v.y - this.y * v.x;
  if (this.p5) {
    return new p5.Vector(this.p5,[x,y,z]);
  } else {
    return new p5.Vector(x,y,z);
  }
};

/**
 * Calculates the Euclidean distance between two points (considering a
 * point as a vector object).
 *
 * @method dist
 * @param  {p5.Vector} v the x, y, and z coordinates of a p5.Vector
 * @return {Number}      the distance
 * @example
 * <div class="norender">
 * <code>
 * var v1 = createVector(1, 0, 0);
 * var v2 = createVector(0, 1, 0);
 *
 * var distance = v1.dist(v2); // distance is 1.4142...
 * </code>
 * </div>
 * <div class="norender">
 * <code>
 * // Static method
 * var v1 = createVector(1, 0, 0);
 * var v2 = createVector(0, 1, 0);
 *
 * var distance = p5.Vector.dist(v1,v2);
 * // distance is 1.4142...
 * </code>
 * </div>
 */
p5.Vector.prototype.dist = function (v) {
  var d = v.copy().sub(this);
  return d.mag();
};

/**
 * Normalize the vector to length 1 (make it a unit vector).
 *
 * @method normalize
 * @return {p5.Vector} normalized p5.Vector
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.normalize();
 * // v's components are set to
 * // [0.4454354, 0.8908708, 0.089087084]
 * </code>
 * </div>
 *
 */
p5.Vector.prototype.normalize = function () {
  return this.mag() === 0 ? this : this.div(this.mag());
};

/**
 * Limit the magnitude of this vector to the value used for the <b>max</b>
 * parameter.
 *
 * @method limit
 * @param  {Number}    max the maximum magnitude for the vector
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.limit(5);
 * // v's components are set to
 * // [2.2271771, 4.4543543, 0.4454354]
 * </code>
 * </div>
 */
p5.Vector.prototype.limit = function (max) {
  var mSq = this.magSq();
  if(mSq > max*max) {
    this.div(Math.sqrt(mSq)); //normalize it
    this.mult(max);
  }
  return this;
};

/**
 * Set the magnitude of this vector to the value used for the <b>len</b>
 * parameter.
 *
 * @method setMag
 * @param  {number}    len the new length for this vector
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.setMag(10);
 * // v's components are set to [6.0, 8.0, 0.0]
 * </code>
 * </div>
 */
p5.Vector.prototype.setMag = function (n) {
  return this.normalize().mult(n);
};

/**
 * Calculate the angle of rotation for this vector (only 2D vectors)
 *
 * @method heading
 * @return {Number} the angle of rotation
 * @example
 * <div class = "norender"><code>
 * function setup() {
 *   var v1 = createVector(30,50);
 *   print(v1.heading()); // 1.0303768265243125
 *
 *   var v1 = createVector(40,50);
 *   print(v1.heading()); // 0.8960553845713439
 *
 *   var v1 = createVector(30,70);
 *   print(v1.heading()); // 1.1659045405098132
 * }
 * </div></code>
 */
p5.Vector.prototype.heading = function () {
  var h = Math.atan2(this.y, this.x);
  if (this.p5) {
    if (this.p5._angleMode === constants.RADIANS) {
      return h;
    } else {
      return polarGeometry.radiansToDegrees(h);
    }
  } else {
    return h;
  }
};

/**
 * Rotate the vector by an angle (only 2D vectors), magnitude remains the
 * same
 *
 * @method rotate
 * @param  {number}    angle the angle of rotation
 * @chainable
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(10.0, 20.0);
 * // v has components [10.0, 20.0, 0.0]
 * v.rotate(HALF_PI);
 * // v's components are set to [-20.0, 9.999999, 0.0]
 * </code>
 * </div>
 */
p5.Vector.prototype.rotate = function (a) {
  var newHeading = this.heading() + a;
  if (this.p5) {
    if (this.p5._angleMode === constants.DEGREES) {
      newHeading = polarGeometry.degreesToRadians(newHeading);
    }
  }
  var mag = this.mag();
  this.x = Math.cos(newHeading) * mag;
  this.y = Math.sin(newHeading) * mag;
  return this;
};

/**
 * Calculates and returns the angle (in radians) between two vectors.
 * @method angleBetween
 * @param  {p5.Vector}    the x, y, and z components of a p5.Vector
 * @return {Number}       the angle between (in radians)
 * @example
 * <div class="norender">
 * <code>
 * var v1 = createVector(1, 0, 0);
 * var v2 = createVector(0, 1, 0);
 *
 * var angle = v1.angleBetween(v2);
 * // angle is PI/2
 * </code>
 * </div>
 */
p5.Vector.prototype.angleBetween = function (v) {
  var angle = Math.acos(this.dot(v) / (this.mag() * v.mag()));
  if (this.p5) {
    if (this.p5._angleMode === constants.DEGREES) {
      angle = polarGeometry.radiansToDegrees(angle);
    }
  }
  return angle;
};

/**
 * Linear interpolate the vector to another vector
 *
 * @method lerp
 * @param  {p5.Vector} x   the x component
 * @param  {p5.Vector} y   the y component
 * @param  {p5.Vector} z   the z component
 * @param  {Number}    amt the amount of interpolation; some value between 0.0
 *                         (old vector) and 1.0 (new vector). 0.1 is very near
 *                         the new vector. 0.5 is halfway in between.
 * @chainable
 */
/**
 * @method lerp
 * @param  {p5.Vector} v   the p5.Vector to lerp to
 * @param  {Number}    amt
 * @chainable
 *
 * @example
 * <div class="norender">
 * <code>
 * var v = createVector(1, 1, 0);
 *
 * v.lerp(3, 3, 0, 0.5); // v now has components [2,2,0]
 * </code>
 * </div>
 *
 * <div class="norender">
 * <code>
 * var v1 = createVector(0, 0, 0);
 * var v2 = createVector(100, 100, 0);
 *
 * var v3 = p5.Vector.lerp(v1, v2, 0.5);
 * // v3 has components [50,50,0]
 * </code>
 * </div>
 */
p5.Vector.prototype.lerp = function (x, y, z, amt) {
  if (x instanceof p5.Vector) {
    return this.lerp(x.x, x.y, x.z, y);
  }
  this.x += (x - this.x) * amt || 0;
  this.y += (y - this.y) * amt || 0;
  this.z += (z - this.z) * amt || 0;
  return this;
};

/**
 * Return a representation of this vector as a float array. This is only
 * for temporary use. If used in any other fashion, the contents should be
 * copied by using the <b>p5.Vector.copy()</b> method to copy into your own
 * array.
 *
 * @method array
 * @return {Number[]} an Array with the 3 values
 * @example
 * <div class = "norender"><code>
 * function setup() {
 *   var v = createVector(20,30);
 *   print(v.array()); // Prints : Array [20, 30, 0]
 * }
 * </div></code>
 * <div class="norender">
 * <code>
 * var v = createVector(10.0, 20.0, 30.0);
 * var f = v.array();
 * print(f[0]); // Prints "10.0"
 * print(f[1]); // Prints "20.0"
 * print(f[2]); // Prints "30.0"
 * </code>
 * </div>
 */
p5.Vector.prototype.array = function () {
  return [this.x || 0, this.y || 0, this.z || 0];
};

/**
 * Equality check against a p5.Vector
 *
 * @method equals
 * @param {Number|p5.Vector|Array} [x] the x component of the vector or a
 *                                     p5.Vector or an Array
 * @param {Number}                 [y] the y component of the vector
 * @param {Number}                 [z] the z component of the vector
 * @return {Boolean} whether the vectors are equals
 * @example
 * <div class = "norender"><code>
 * v1 = createVector(5,10,20);
 * v2 = createVector(5,10,20);
 * v3 = createVector(13,10,19);
 *
 * print(v1.equals(v2.x,v2.y,v2.z)); // true
 * print(v1.equals(v3.x,v3.y,v3.z)); // false
 * </div></code>
 * <div class="norender">
 * <code>
 * var v1 = createVector(10.0, 20.0, 30.0);
 * var v2 = createVector(10.0, 20.0, 30.0);
 * var v3 = createVector(0.0, 0.0, 0.0);
 * print (v1.equals(v2)) // true
 * print (v1.equals(v3)) // false
 * </code>
 * </div>
 */
p5.Vector.prototype.equals = function (x, y, z) {
  var a, b, c;
  if (x instanceof p5.Vector) {
    a = x.x || 0;
    b = x.y || 0;
    c = x.z || 0;
  } else if (x instanceof Array) {
    a = x[0] || 0;
    b = x[1] || 0;
    c = x[2] || 0;
  } else {
    a = x || 0;
    b = y || 0;
    c = z || 0;
  }
  return this.x === a && this.y === b && this.z === c;
};


// Static Methods


/**
 * Make a new 2D unit vector from an angle
 *
 * @method fromAngle
 * @static
 * @param {Number}     angle the desired angle
 * @return {p5.Vector}       the new p5.Vector object
 * @example
 * <div>
 * <code>
 * function draw() {
 *   background (200);
 *
 *   // Create a variable, proportional to the mouseX,
 *   // varying from 0-360, to represent an angle in degrees.
 *   angleMode(DEGREES);
 *   var myDegrees = map(mouseX, 0,width, 0,360);
 *
 *   // Display that variable in an onscreen text.
 *   // (Note the nfc() function to truncate additional decimal places,
 *   // and the "\xB0" character for the degree symbol.)
 *   var readout = "angle = " + nfc(myDegrees,1,1) + "\xB0"
 *   noStroke();
 *   fill (0);
 *   text (readout, 5, 15);
 *
 *   // Create a p5.Vector using the fromAngle function,
 *   // and extract its x and y components.
 *   var v = p5.Vector.fromAngle(radians(myDegrees));
 *   var vx = v.x;
 *   var vy = v.y;
 *
 *   push();
 *   translate (width/2, height/2);
 *   noFill();
 *   stroke (150);
 *   line (0,0, 30,0);
 *   stroke (0);
 *   line (0,0, 30*vx, 30*vy);
 *   pop()
 * }
 * </code>
 * </div>
 */
p5.Vector.fromAngle = function(angle) {
  if (this.p5) {
    if (this.p5._angleMode === constants.DEGREES) {
      angle = polarGeometry.degreesToRadians(angle);
    }
  }
  if (this.p5) {
    return new p5.Vector(this.p5,[Math.cos(angle),Math.sin(angle),0]);
  } else {
    return new p5.Vector(Math.cos(angle),Math.sin(angle),0);
  }
};

/**
 * Make a new 2D unit vector from a random angle
 *
 * @method random2D
 * @static
 * @return {p5.Vector} the new p5.Vector object
 * @example
 * <div class="norender">
 * <code>
 * var v = p5.Vector.random2D();
 * // May make v's attributes something like:
 * // [0.61554617, -0.51195765, 0.0] or
 * // [-0.4695841, -0.14366731, 0.0] or
 * // [0.6091097, -0.22805278, 0.0]
 * </code>
 * </div>
 */
p5.Vector.random2D = function () {
  var angle;
  // A lot of nonsense to determine if we know about a
  // p5 sketch and whether we should make a random angle in degrees or radians
  if (this.p5) {
    if (this.p5._angleMode === constants.DEGREES) {
      angle = this.p5.random(360);
    } else {
      angle = this.p5.random(constants.TWO_PI);
    }
  } else {
    angle = Math.random()*Math.PI*2;
  }
  return this.fromAngle(angle);
};

/**
 * Make a new random 3D unit vector.
 *
 * @method random3D
 * @static
 * @return {p5.Vector} the new p5.Vector object
 * @example
 * <div class="norender">
 * <code>
 * var v = p5.Vector.random3D();
 * // May make v's attributes something like:
 * // [0.61554617, -0.51195765, 0.599168] or
 * // [-0.4695841, -0.14366731, -0.8711202] or
 * // [0.6091097, -0.22805278, -0.7595902]
 * </code>
 * </div>
 */
p5.Vector.random3D = function () {
  var angle,vz;
  // If we know about p5
  if (this.p5) {
    angle = this.p5.random(0,constants.TWO_PI);
    vz = this.p5.random(-1,1);
  } else {
    angle = Math.random()*Math.PI*2;
    vz = Math.random()*2-1;
  }
  var vx = Math.sqrt(1-vz*vz)*Math.cos(angle);
  var vy = Math.sqrt(1-vz*vz)*Math.sin(angle);
  if (this.p5) {
    return new p5.Vector(this.p5,[vx,vy,vz]);
  } else {
    return new p5.Vector(vx,vy,vz);
  }
};

// Adds two vectors together and returns a new one.
/**
 * @method add
 * @static
 * @param  {p5.Vector} v1 a p5.Vector to add
 * @param  {p5.Vector} v2 a p5.Vector to add
 * @param  {p5.Vector} target the vector to receive the result
 */
/**
 * @method add
 * @static
 * @param  {p5.Vector} v1
 * @param  {p5.Vector} v2
 * @return {p5.Vector} the resulting p5.Vector
 *
 */

p5.Vector.add = function (v1, v2, target) {
  if (!target) {
    target = v1.copy();
  } else {
    target.set(v1);
  }
  target.add(v2);
  return target;
};

/*
 * Subtracts one p5.Vector from another and returns a new one.  The second
 * vector (v2) is subtracted from the first (v1), resulting in v1-v2.
 */
/**
 * @method sub
 * @static
 * @param  {p5.Vector} v1 a p5.Vector to subtract from
 * @param  {p5.Vector} v2 a p5.Vector to subtract
 * @param  {p5.Vector} target if undefined a new vector will be created
 */
/**
 * @method sub
 * @static
 * @param  {p5.Vector} v1
 * @param  {p5.Vector} v2
 * @return {p5.Vector} the resulting p5.Vector
 */

p5.Vector.sub = function (v1, v2, target) {
  if (!target) {
    target = v1.copy();
  } else {
    target.set(v1);
  }
  target.sub(v2);
  return target;
};


/**
 * Multiplies a vector by a scalar and returns a new vector.
 */
/**
 * @method mult
 * @static
 * @param  {p5.Vector} v the vector to multiply
 * @param  {Number}  n
 * @param  {p5.Vector} target if undefined a new vector will be created
 */
/**
 * @method mult
 * @static
 * @param  {p5.Vector} v
 * @param  {Number}  n
 * @return {p5.Vector}  the resulting new p5.Vector
 */
p5.Vector.mult = function (v, n, target) {
  if (!target) {
    target = v.copy();
  } else {
    target.set(v);
  }
  target.mult(n);
  return target;
};

/**
 * Divides a vector by a scalar and returns a new vector.
 */
/**
 * @method div
 * @static
 * @param  {p5.Vector} v the vector to divide
 * @param  {Number}  n
 * @param  {p5.Vector} target if undefined a new vector will be created
 */
/**
 * @method div
 * @static
 * @param  {p5.Vector} v
 * @param  {Number}  n
 * @return {p5.Vector} the resulting new p5.Vector
 */
p5.Vector.div = function (v, n, target) {
  if (!target) {
    target = v.copy();
  } else {
    target.set(v);
  }
  target.div(n);
  return target;
};


/**
 * Calculates the dot product of two vectors.
 */
/**
 * @method dot
 * @static
 * @param  {p5.Vector} v1 the first p5.Vector
 * @param  {p5.Vector} v2 the second p5.Vector
 * @return {Number}     the dot product
 */
p5.Vector.dot = function (v1, v2) {
  return v1.dot(v2);
};

/**
 * Calculates the cross product of two vectors.
 */
/**
 * @method cross
 * @static
 * @param  {p5.Vector} v1 the first p5.Vector
 * @param  {p5.Vector} v2 the second p5.Vector
 * @return {Number}     the cross product
 */
p5.Vector.cross = function (v1, v2) {
  return v1.cross(v2);
};

/**
 * Calculates the Euclidean distance between two points (considering a
 * point as a vector object).
 */
/**
 * @method dist
 * @static
 * @param  {p5.Vector} v1 the first p5.Vector
 * @param  {p5.Vector} v2 the second p5.Vector
 * @return {Number}     the distance
 */
p5.Vector.dist = function (v1,v2) {
  return v1.dist(v2);
};

/**
 * Linear interpolate a vector to another vector and return the result as a
 * new vector.
 */
/**
 * @method lerp
 * @static
 * @param {p5.Vector} v1
 * @param {p5.Vector} v2
 * @param {Number} amt
 * @param {p5.Vector} target if undefined a new vector will be created
 */
/**
 * @method lerp
 * @static
 * @param {p5.Vector} v1
 * @param {p5.Vector} v2
 * @param {Number} amt
 * @return {Number}      the lerped value
 */
p5.Vector.lerp = function (v1, v2, amt, target) {
  if (!target) {
    target = v1.copy();
  } else {
    target.set(v1);
  }
  target.lerp(v2, amt);
  return target;
};

/**
 * @method mag
 * @param {p5.Vector} vecT the vector to return the magnitude of
 * @return {Number}        the magnitude of vecT
 * @static
 */
p5.Vector.mag = function (vecT){
  var x = vecT.x,
    y = vecT.y,
    z = vecT.z;
  var magSq = x * x + y * y + z * z;
  return Math.sqrt(magSq);
};

module.exports = p5.Vector;

},{"../core/constants":53,"../core/core":54,"./polargeometry":86}],86:[function(_dereq_,module,exports){

module.exports = {

  degreesToRadians: function(x) {
    return 2 * Math.PI * x / 360;
  },

  radiansToDegrees: function(x) {
    return 360 * x / (2 * Math.PI);
  }

};

},{}],88:[function(_dereq_,module,exports){
/**
 * @module Math
 * @submodule Trigonometry
 * @for p5
 * @requires core
 * @requires polargeometry
 * @requires constants
 */

'use strict';

var p5 = _dereq_('../core/core');
var polarGeometry = _dereq_('./polargeometry');
var constants = _dereq_('../core/constants');

p5.prototype._angleMode = constants.RADIANS;

/**
 * The inverse of cos(), returns the arc cosine of a value. This function
 * expects the values in the range of -1 to 1 and values are returned in
 * the range 0 to PI (3.1415927).
 *
 * @method acos
 * @param  {Number} value the value whose arc cosine is to be returned
 * @return {Number}       the arc cosine of the given value
 *
 * @example
 * <div class= “norender">
 * <code>
 * var a = PI;
 * var c = cos(a);
 * var ac = acos(c);
 * // Prints: "3.1415927 : -1.0 : 3.1415927"
 * print(a + " : " + c + " : " +  ac);
 * </code>
 * </div>
 *
 * <div class= “norender">
 * <code>
 * var a = PI + PI/4.0;
 * var c = cos(a);
 * var ac = acos(c);
 * // Prints: "3.926991 : -0.70710665 : 2.3561943"
 * print(a + " : " + c + " : " +  ac);
 * </code>
 * </div>
 */
p5.prototype.acos = function(ratio) {
  if (this._angleMode === constants.RADIANS) {
    return Math.acos(ratio);
  } else {
    return polarGeometry.radiansToDegrees(Math.acos(ratio));
  }
};

/**
 * The inverse of sin(), returns the arc sine of a value. This function
 * expects the values in the range of -1 to 1 and values are returned
 * in the range -PI/2 to PI/2.
 *
 * @method asin
 * @param  {Number} value the value whose arc sine is to be returned
 * @return {Number}       the arc sine of the given value
 *
 * @example
 * <div class= “norender">
 * <code>
 * var a = PI + PI/3;
 * var s = sin(a);
 * var as = asin(s);
 * // Prints: "1.0471976 : 0.86602545 : 1.0471976"
 * print(a + " : " + s + " : " +  as);
 * </code>
 * </div>
 *
 * <div class= “norender">
 * <code>
 * var a = PI + PI/3.0;
 * var s = sin(a);
 * var as = asin(s);
 * // Prints: "4.1887903 : -0.86602545 : -1.0471976"
 * print(a + " : " + s + " : " +  as);
 * </code>
 * </div>
 *
 */
p5.prototype.asin = function(ratio) {
  if (this._angleMode === constants.RADIANS) {
    return Math.asin(ratio);
  } else {
    return polarGeometry.radiansToDegrees(Math.asin(ratio));
  }
};

/**
 * The inverse of tan(), returns the arc tangent of a value. This function
 * expects the values in the range of -Infinity to Infinity (exclusive) and
 * values are returned in the range -PI/2 to PI/2.
 *
 * @method atan
 * @param  {Number} value the value whose arc tangent is to be returned
 * @return {Number}       the arc tangent of the given value
 *
 * @example
 * <div class= “norender">
 * <code>
 * var a = PI + PI/3;
 * var t = tan(a);
 * var at = atan(t);
 * // Prints: "1.0471976 : 1.7320509 : 1.0471976"
 * print(a + " : " + t + " : " +  at);
 * </code>
 * </div>
 *
 * <div class= “norender">
 * <code>
 * var a = PI + PI/3.0;
 * var t = tan(a);
 * var at = atan(t);
 * // Prints: "4.1887903 : 1.7320513 : 1.0471977"
 * print(a + " : " + t + " : " +  at);
 * </code>
 * </div>
 *
 */
p5.prototype.atan = function(ratio) {
  if (this._angleMode === constants.RADIANS) {
    return Math.atan(ratio);
  } else {
    return polarGeometry.radiansToDegrees(Math.atan(ratio));
  }
};

/**
 * Calculates the angle (in radians) from a specified point to the coordinate
 * origin as measured from the positive x-axis. Values are returned as a
 * float in the range from PI to -PI. The atan2() function is most often used
 * for orienting geometry to the position of the cursor.
 * <br><br>
 * Note: The y-coordinate of the point is the first parameter, and the
 * x-coordinate is the second parameter, due the the structure of calculating
 * the tangent.
 *
 * @method atan2
 * @param  {Number} y y-coordinate of the point
 * @param  {Number} x x-coordinate of the point
 * @return {Number}   the arc tangent of the given point
 *
 * @example
 * <div>
 * <code>
 * function draw() {
 *   background(204);
 *   translate(width/2, height/2);
 *   var a = atan2(mouseY-height/2, mouseX-width/2);
 *   rotate(a);
 *   rect(-30, -5, 60, 10);
 * }
 * </code>
 * </div>
 *
 * @alt
 * 60 by 10 rect at center of canvas rotates with mouse movements
 *
 */
p5.prototype.atan2 = function (y, x) {
  if (this._angleMode === constants.RADIANS) {
    return Math.atan2(y, x);
  } else {
    return polarGeometry.radiansToDegrees(Math.atan2(y, x));
  }
};

/**
 * Calculates the cosine of an angle. This function takes into account the
 * current angleMode. Values are returned in the range -1 to 1.
 *
 * @method cos
 * @param  {Number} angle the angle
 * @return {Number}       the cosine of the angle
 *
 * @example
 * <div>
 * <code>
 * var a = 0.0;
 * var inc = TWO_PI/25.0;
 * for (var i = 0; i < 25; i++) {
 *   line(i*4, 50, i*4, 50+cos(a)*40.0);
 *   a = a + inc;
 * }
 * </code>
 * </div>
 *
 * @alt
 * vertical black lines form wave patterns, extend-down on left and right side
 *
 */
p5.prototype.cos = function(angle) {
  if (this._angleMode === constants.RADIANS) {
    return Math.cos(angle);
  } else {
    return Math.cos(this.radians(angle));
  }
};

/**
 * Calculates the sine of an angle. This function takes into account the
 * current angleMode. Values are returned in the range -1 to 1.
 *
 * @method sin
 * @param  {Number} angle the angle
 * @return {Number}       the sine of the angle
 *
 * @example
 * <div>
 * <code>
 * var a = 0.0;
 * var inc = TWO_PI/25.0;
 * for (var i = 0; i < 25; i++) {
 *   line(i*4, 50, i*4, 50+sin(a)*40.0);
 *   a = a + inc;
 * }
 * </code>
 * </div>
 *
 * @alt
 * vertical black lines extend down and up from center to form wave pattern
 *
 */
p5.prototype.sin = function(angle) {
  if (this._angleMode === constants.RADIANS) {
    return Math.sin(angle);
  } else {
    return Math.sin(this.radians(angle));
  }
};

/**
 * Calculates the tangent of an angle. This function takes into account
 * the current angleMode. Values are returned in the range -1 to 1.
 *
 * @method tan
 * @param  {Number} angle the angle
 * @return {Number}       the tangent of the angle
 *
 * @example
 * <div>
 * <code>
 *   var a = 0.0;
 *   var inc = TWO_PI/50.0;
 *   for (var i = 0; i < 100; i = i+2) {
 *     line(i, 50, i, 50+tan(a)*2.0);
 *     a = a + inc;
 *   }
 * </code>
 *
 *
 * @alt
 * vertical black lines end down and up from center to form spike pattern
 *
 */
p5.prototype.tan = function(angle) {
  if (this._angleMode === constants.RADIANS) {
    return Math.tan(angle);
  } else {
    return Math.tan(this.radians(angle));
  }
};

/**
 * Converts a radian measurement to its corresponding value in degrees.
 * Radians and degrees are two ways of measuring the same thing. There are
 * 360 degrees in a circle and 2*PI radians in a circle. For example,
 * 90° = PI/2 = 1.5707964.
 *
 * @method degrees
 * @param  {Number} radians the radians value to convert to degrees
 * @return {Number}         the converted angle
 *
 *
 * @example
 * <div class= “norender">
 * <code>
 * var rad = PI/4;
 * var deg = degrees(rad);
 * print(rad + " radians is " + deg + " degrees");
 * // Prints: 0.7853981633974483 radians is 45 degrees
 * </code>
 * </div>
 *
 */
p5.prototype.degrees = function(angle) {
  return polarGeometry.radiansToDegrees(angle);
};

/**
 * Converts a degree measurement to its corresponding value in radians.
 * Radians and degrees are two ways of measuring the same thing. There are
 * 360 degrees in a circle and 2*PI radians in a circle. For example,
 * 90° = PI/2 = 1.5707964.
 *
 * @method radians
 * @param  {Number} degrees the degree value to convert to radians
 * @return {Number}         the converted angle
 *
 * @example
 * <div class= “norender">
 * <code>
 * var deg = 45.0;
 * var rad = radians(deg);
 * print(deg + " degrees is " + rad + " radians");
 * // Prints: 45 degrees is 0.7853981633974483 radians
 * </code>
 * </div>
 */
p5.prototype.radians = function(angle) {
  return polarGeometry.degreesToRadians(angle);
};

/**
 * Sets the current mode of p5 to given mode. Default mode is RADIANS.
 *
 * @method angleMode
 * @param {Constant} mode either RADIANS or DEGREES
 *
 * @example
 * <div>
 * <code>
 * function draw(){
 *   background(204);
 *   angleMode(DEGREES); // Change the mode to DEGREES
 *   var a = atan2(mouseY-height/2, mouseX-width/2);
 *   translate(width/2, height/2);
 *   push();
 *   rotate(a);
 *   rect(-20, -5, 40, 10); // Larger rectangle is rotating in degrees
 *   pop();
 *   angleMode(RADIANS); // Change the mode to RADIANS
 *   rotate(a); // var a stays the same
 *   rect(-40, -5, 20, 10); // Smaller rectangle is rotating in radians
 * }
 * </code>
 * </div>
 *
 * @alt
 * 40 by 10 rect in center rotates with mouse moves. 20 by 10 rect moves faster.
 *
 *
 */
p5.prototype.angleMode = function(mode) {
  if (mode === constants.DEGREES || mode === constants.RADIANS) {
    this._angleMode = mode;
  }
};

module.exports = p5;

},{"../core/constants":53,"../core/core":54,"./polargeometry":86}],96:[function(_dereq_,module,exports){
/**
 * @module Lights, Camera
 * @submodule Camera
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');

/**
 * Sets camera position
 * @method camera
 * @param  {Number} x  camera position value on x axis
 * @param  {Number} y  camera position value on y axis
 * @param  {Number} z  camera position value on z axis
 * @return {p5}        the p5 object
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *  //move the camera away from the plane by a sin wave
 *  camera(0, 0, sin(frameCount * 0.01) * 100);
 *  plane(120, 120);
 * }
 * </code>
 * </div>
 *
 * @alt
 * blue square shrinks in size grows to fill canvas. disappears then loops.
 *
 */
p5.prototype.camera = function(x, y, z){
  //what it manipulates is the model view matrix
  this._renderer.translate(-x, -y, -z);
};

/**
 * Sets perspective camera
 * @method  perspective
 * @param  {Number} fovy   camera frustum vertical field of view,
 *                         from bottom to top of view, in degrees
 * @param  {Number} aspect camera frustum aspect ratio
 * @param  {Number} near   frustum near plane length
 * @param  {Number} far    frustum far plane length
 * @return {p5}            the p5 object
 * @example
 * <div>
 * <code>
 * //drag mouse to toggle the world!
 * //you will see there's a vanish point
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *   var fov = 60 / 180 * PI;
 *   var cameraZ = (height/2.0) / tan(fov/2.0);
 *   perspective(60 / 180 * PI, width/height, cameraZ * 0.1, cameraZ * 10);
 * }
 * function draw(){
 *  background(200);
 *  orbitControl();
 *  for(var i = -1; i < 2; i++){
 *     for(var j = -2; j < 3; j++){
 *       push();
 *       translate(i*160, 0, j*160);
 *       box(40, 40, 40);
 *       pop();
 *     }
 *   }
 * }
 * </code>
 * </div>
 *
 * @alt
 * colored 3d boxes toggleable with mouse position
 *
 */
p5.prototype.perspective = function(fovy,aspect,near,far) {
  fovy = fovy || (60 / 180 * this.PI);
  aspect = aspect || (this.width/this.height);
  near = near || ((this.height/2.0) / this.tan(fovy/2.0) * 0.1);
  far = far || ((this.height/2.0) / this.tan(fovy/2.0) * 10);
  this._renderer.uPMatrix = p5.Matrix.identity();
  this._renderer.uPMatrix.perspective(fovy,aspect,near,far);
  this._renderer._curCamera = 'custom';
};

/**
 * Setup ortho camera
 * @method  ortho
 * @param  {Number} left   camera frustum left plane
 * @param  {Number} right  camera frustum right plane
 * @param  {Number} bottom camera frustum bottom plane
 * @param  {Number} top    camera frustum top plane
 * @param  {Number} near   camera frustum near plane
 * @param  {Number} far    camera frustum far plane
 * @return {p5}            the p5 object
 * @example
 * <div>
 * <code>
 * //drag mouse to toggle the world!
 * //there's no vanish point
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *   ortho(-width/2, width/2, height/2, -height/2, 0, 500);
 * }
 * function draw(){
 *  background(200);
 *  orbitControl();
 *  for(var i = -1; i < 2; i++){
 *     for(var j = -2; j < 3; j++){
 *       push();
 *       translate(i*160, 0, j*160);
 *       box(40, 40, 40);
 *       pop();
 *     }
 *   }
 * }
 * </code>
 * </div>
 *
 * @alt
 * 3 3d boxes, reveal several more boxes on 3d plane when mouse used to toggle
 *
 */
p5.prototype.ortho = function(left,right,bottom,top,near,far) {
  left = left || (-this.width/2);
  right = right || (this.width/2);
  bottom = bottom || (-this.height/2);
  top = top || (this.height/2);
  near = near || 0;
  far = far || Math.max(this.width, this.height);
  this._renderer.uPMatrix = p5.Matrix.identity();
  this._renderer.uPMatrix.ortho(left,right,bottom,top,near,far);
  this._renderer._curCamera = 'custom';
};

module.exports = p5;

},{"../core/core":54}],97:[function(_dereq_,module,exports){
'use strict';

var p5 = _dereq_('../core/core');

//@TODO: implement full orbit controls including
//pan, zoom, quaternion rotation, etc.
p5.prototype.orbitControl = function(){
  if(this.mouseIsPressed){
    this.rotateY((this.mouseX - this.width / 2) / (this.width / 2));
    this.rotateX((this.mouseY - this.height / 2) / (this.width / 2));
  }
  return this;
};

module.exports = p5;
},{"../core/core":54}],98:[function(_dereq_,module,exports){
/**
 * @module Lights, Camera
 * @submodule Lights
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');

/**
 * Creates an ambient light with a color
 *
 * @method ambientLight
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method ambientLight
 * @param  {String}        value   a color string
 * @param  {Number}        [alpha]
 * @chainable
 */

/**
 * @method ambientLight
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 * @chainable
 */

/**
 * @method ambientLight
 * @param  {p5.Color}      color   the ambient light color
 * @param  {Number}        [alpha]
 * @chainable
 *
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *   background(0);
 *   ambientLight(150);
 *   ambientMaterial(250);
 *   sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * nothing displayed
 *
 */
p5.prototype.ambientLight = function(v1, v2, v3, a){
  var gl = this._renderer.GL;
  var shaderProgram = this._renderer._getShader(
    'lightVert', 'lightTextureFrag');

  gl.useProgram(shaderProgram);
  shaderProgram.uAmbientColor = gl.getUniformLocation(
    shaderProgram,
    'uAmbientColor[' + this._renderer.ambientLightCount + ']');

  var color = this._renderer._pInst.color.apply(
    this._renderer._pInst, arguments);
  var colors = color._array;

  gl.uniform3f( shaderProgram.uAmbientColor,
    colors[0], colors[1], colors[2]);

  //in case there's no material color for the geometry
  shaderProgram.uMaterialColor = gl.getUniformLocation(
    shaderProgram, 'uMaterialColor' );
  gl.uniform4f( shaderProgram.uMaterialColor, 1, 1, 1, 1);

  this._renderer.ambientLightCount ++;
  shaderProgram.uAmbientLightCount =
    gl.getUniformLocation(shaderProgram, 'uAmbientLightCount');
  gl.uniform1i(shaderProgram.uAmbientLightCount,
    this._renderer.ambientLightCount);

  return this;
};

/**
 * Creates a directional light with a color and a direction
 * @method  directionalLight
 * @param  {Number|Array|String|p5.Color} v1   gray value,
 * red or hue value (depending on the current color mode),
 * or color Array, or CSS color string
 * @param  {Number}          [v2] optional: green or saturation value
 * @param  {Number}          [v3] optional: blue or brightness value
 * @param  {Number}          [a]  optional: opacity
 * @param  {Number|p5.Vector} x   x axis direction or a p5.Vector
 * @param  {Number}          [y]  optional: y axis direction
 * @param  {Number}          [z]  optional: z axis direction
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *   background(0);
 *   //move your mouse to change light direction
 *   var dirX = (mouseX / width - 0.5) *2;
 *   var dirY = (mouseY / height - 0.5) *(-2);
 *   directionalLight(250, 250, 250, dirX, dirY, 0.25);
 *   ambientMaterial(250);
 *   sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * light source on canvas changeable with mouse position
 *
 */
p5.prototype.directionalLight = function(v1, v2, v3, a, x, y, z) {
  var gl = this._renderer.GL;
  var shaderProgram = this._renderer._getShader(
    'lightVert', 'lightTextureFrag');

  gl.useProgram(shaderProgram);
  shaderProgram.uDirectionalColor = gl.getUniformLocation(
    shaderProgram,
    'uDirectionalColor[' + this._renderer.directionalLightCount + ']');

  //@TODO: check parameters number
  var color = this._renderer._pInst.color.apply(
    this._renderer._pInst, [v1, v2, v3]);
  var colors = color._array;

  gl.uniform3f( shaderProgram.uDirectionalColor,
    colors[0], colors[1], colors[2]);

  var _x, _y, _z;

  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if(typeof args[args.length-1] === 'number'){
    _x = args[args.length-3];
    _y = args[args.length-2];
    _z = args[args.length-1];

  }else{
    try{
      _x = args[args.length-1].x;
      _y = args[args.length-1].y;
      _z = args[args.length-1].z;
    }
    catch(error){
      throw error;
    }
  }

  shaderProgram.uLightingDirection = gl.getUniformLocation(
    shaderProgram,
    'uLightingDirection[' + this._renderer.directionalLightCount + ']');
  gl.uniform3f( shaderProgram.uLightingDirection, _x, _y, _z);

  //in case there's no material color for the geometry
  shaderProgram.uMaterialColor = gl.getUniformLocation(
    shaderProgram, 'uMaterialColor' );
  gl.uniform4f( shaderProgram.uMaterialColor, 1, 1, 1, 1);

  this._renderer.directionalLightCount ++;
  shaderProgram.uDirectionalLightCount =
    gl.getUniformLocation(shaderProgram, 'uDirectionalLightCount');
  gl.uniform1i(shaderProgram.uDirectionalLightCount,
    this._renderer.directionalLightCount);

  return this;
};

/**
 * Creates a point light with a color and a light position
 * @method  pointLight
 * @param  {Number|Array|String|p5.Color} v1   gray value,
 * red or hue value (depending on the current color mode),
 * or color Array, or CSS color string
 * @param  {Number}          [v2] optional: green or saturation value
 * @param  {Number}          [v3] optional: blue or brightness value
 * @param  {Number}          [a]  optional: opacity
 * @param  {Number|p5.Vector} x   x axis position or a p5.Vector
 * @param  {Number}          [y]  optional: y axis position
 * @param  {Number}          [z]  optional: z axis position
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *   background(0);
 *   //move your mouse to change light position
 *   var locY = (mouseY / height - 0.5) *(-2);
 *   var locX = (mouseX / width - 0.5) *2;
 *   //to set the light position,
 *   //think of the world's coordinate as:
 *   // -1,1 -------- 1,1
 *   //   |            |
 *   //   |            |
 *   //   |            |
 *   // -1,-1---------1,-1
 *   pointLight(250, 250, 250, locX, locY, 0);
 *   ambientMaterial(250);
 *   sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * spot light on canvas changes position with mouse
 *
 */
p5.prototype.pointLight = function(v1, v2, v3, a, x, y, z) {
  var gl = this._renderer.GL;
  var shaderProgram = this._renderer._getShader(
    'lightVert', 'lightTextureFrag');

  gl.useProgram(shaderProgram);
  shaderProgram.uPointLightColor = gl.getUniformLocation(
    shaderProgram,
    'uPointLightColor[' + this._renderer.pointLightCount + ']');

  //@TODO: check parameters number
  var color = this._renderer._pInst.color.apply(
    this._renderer._pInst, [v1, v2, v3]);
  var colors = color._array;

  gl.uniform3f( shaderProgram.uPointLightColor,
    colors[0], colors[1], colors[2]);

  var _x, _y, _z;

  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  if(typeof args[args.length-1] === 'number'){
    _x = args[args.length-3];
    _y = args[args.length-2];
    _z = args[args.length-1];

  }else{
    try{
      _x = args[args.length-1].x;
      _y = args[args.length-1].y;
      _z = args[args.length-1].z;
    }
    catch(error){
      throw error;
    }
  }

  shaderProgram.uPointLightLocation = gl.getUniformLocation(
    shaderProgram,
    'uPointLightLocation[' + this._renderer.pointLightCount + ']');
  gl.uniform3f( shaderProgram.uPointLightLocation, _x, _y, _z);

  //in case there's no material color for the geometry
  shaderProgram.uMaterialColor = gl.getUniformLocation(
    shaderProgram, 'uMaterialColor' );
  gl.uniform4f( shaderProgram.uMaterialColor, 1, 1, 1, 1);

  this._renderer.pointLightCount ++;
  shaderProgram.uPointLightCount =
    gl.getUniformLocation(shaderProgram, 'uPointLightCount');
  gl.uniform1i(shaderProgram.uPointLightCount,
    this._renderer.pointLightCount);

  return this;
};

module.exports = p5;

},{"../core/core":54}],99:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule 3D Models
 * @for p5
 * @requires core
 * @requires p5.Geometry
 */

'use strict';

var p5 = _dereq_('../core/core');
_dereq_('./p5.Geometry');

/**
 * Load a 3d model from an OBJ file.
 * <br><br>
 * One of the limitations of the OBJ format is that it doesn't have a built-in
 * sense of scale. This means that models exported from different programs might
 * be very different sizes. If your model isn't displaying, try calling
 * loadModel() with the normalized parameter set to true. This will resize the
 * model to a scale appropriate for p5. You can also make additional changes to
 * the final size of your model with the scale() function.
 *
 * @method loadModel
 * @param  {String} path              Path of the model to be loaded
 * @param  {Boolean} normalize        If true, scale the model to a
 *                                      standardized size when loading
 * @param  {function(p5.Geometry)} [successCallback] Function to be called
 *                                     once the model is loaded. Will be passed
 *                                     the 3D model object.
 * @param  {function(Event)} [failureCallback] called with event error if
 *                                         the image fails to load.
 * @return {p5.Geometry} the p5.Geometry object
 */
/**
 * @method loadModel
 * @param  {String} path
 * @param  {function(p5.Geometry)} [successCallback]
 * @param  {function(Event)} [failureCallback]
 * @return {p5.Geometry} the p5.Geometry object
 * @example
 * <div>
 * <code>
 * //draw a spinning teapot
 * var teapot;
 *
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *
 *   teapot = loadModel('assets/teapot.obj');
 * }
 *
 * function draw(){
 *   background(200);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   model(teapot);
 * }
 * </code>
 * </div>
 *
 * @alt
 * Vertically rotating 3-d teapot with red, green and blue gradient.
 *
 */
p5.prototype.loadModel = function () {
  var path = arguments[0];
  var normalize;
  var successCallback;
  var failureCallback;
  if(typeof arguments[1] === 'boolean') {
    normalize = arguments[1];
    successCallback = arguments[2];
    failureCallback = arguments[3];
  } else {
    normalize = false;
    successCallback = arguments[1];
    failureCallback = arguments[2];
  }

  var model = new p5.Geometry();
  model.gid = path + '|' + normalize;
  this.loadStrings(path, function(strings) {
    parseObj(model, strings);

    if (normalize) {
      model.normalize();
    }

    if (typeof successCallback === 'function') {
      successCallback(model);
    }
  }.bind(this), failureCallback);

  return model;
};

/**
 * Parse OBJ lines into model. For reference, this is what a simple model of a
 * square might look like:
 *
 * v -0.5 -0.5 0.5
 * v -0.5 -0.5 -0.5
 * v -0.5 0.5 -0.5
 * v -0.5 0.5 0.5
 *
 * f 4 3 2 1
 */
function parseObj( model, lines ) {
  // OBJ allows a face to specify an index for a vertex (in the above example),
  // but it also allows you to specify a custom combination of vertex, UV
  // coordinate, and vertex normal. So, "3/4/3" would mean, "use vertex 3 with
  // UV coordinate 4 and vertex normal 3". In WebGL, every vertex with different
  // parameters must be a different vertex, so loadedVerts is used to
  // temporarily store the parsed vertices, normals, etc., and indexedVerts is
  // used to map a specific combination (keyed on, for example, the string
  // "3/4/3"), to the actual index of the newly created vertex in the final
  // object.
  var loadedVerts = {'v' : [],
                    'vt' : [],
                    'vn' : []};
  var indexedVerts = {};

  for (var line = 0; line < lines.length; ++line) {
    // Each line is a separate object (vertex, face, vertex normal, etc)
    // For each line, split it into tokens on whitespace. The first token
    // describes the type.
    var tokens = lines[line].trim().split(/\b\s+/);

    if (tokens.length > 0) {
      if (tokens[0] === 'v' || tokens[0] === 'vn') {
        // Check if this line describes a vertex or vertex normal.
        // It will have three numeric parameters.
        var vertex = new p5.Vector(parseFloat(tokens[1]),
                                   parseFloat(tokens[2]),
                                   parseFloat(tokens[3]));
        loadedVerts[tokens[0]].push(vertex);
      } else if (tokens[0] === 'vt') {
        // Check if this line describes a texture coordinate.
        // It will have two numeric parameters.
        var texVertex = [parseFloat(tokens[1]), parseFloat(tokens[2])];
        loadedVerts[tokens[0]].push(texVertex);
      } else if (tokens[0] === 'f') {
        // Check if this line describes a face.
        // OBJ faces can have more than three points. Triangulate points.
        for (var tri = 3; tri < tokens.length; ++tri) {
          var face = [];

          var vertexTokens = [1, tri - 1, tri];

          for (var tokenInd = 0; tokenInd < vertexTokens.length; ++tokenInd) {
            // Now, convert the given token into an index
            var vertString = tokens[vertexTokens[tokenInd]];
            var vertIndex = 0;

            // TODO: Faces can technically use negative numbers to refer to the
            // previous nth vertex. I haven't seen this used in practice, but
            // it might be good to implement this in the future.

            if (indexedVerts[vertString] !== undefined) {
              vertIndex = indexedVerts[vertString];
            } else {
              var vertParts = vertString.split('/');
              for (var i = 0; i < vertParts.length; i++) {
                vertParts[i] = parseInt(vertParts[i]) - 1;
              }

              vertIndex = indexedVerts[vertString] = model.vertices.length;
              model.vertices.push(loadedVerts.v[vertParts[0]].copy());
              if (loadedVerts.vt[vertParts[1]]) {
                model.uvs.push(loadedVerts.vt[vertParts[1]].slice());
              } else {
                model.uvs.push([0, 0]);
              }

              if (loadedVerts.vn[vertParts[2]]) {
                model.vertexNormals.push(loadedVerts.vn[vertParts[2]].copy());
              }
            }

            face.push(vertIndex);
          }

          model.faces.push(face);
        }
      }
    }
  }

  // If the model doesn't have normals, compute the normals
  if(model.vertexNormals.length === 0) {
    model.computeNormals();
  }

  return model;
}

/**
 * Render a 3d model to the screen.
 *
 * @method model
 * @param  {p5.Geometry} model Loaded 3d model to be rendered
 * @example
 * <div>
 * <code>
 * //draw a spinning teapot
 * var teapot;
 *
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *
 *   teapot = loadModel('assets/teapot.obj');
 * }
 *
 * function draw(){
 *   background(200);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   model(teapot);
 * }
 * </code>
 * </div>
 *
 * @alt
 * Vertically rotating 3-d teapot with red, green and blue gradient.
 *
 */
p5.prototype.model = function ( model ) {
  if (model.vertices.length > 0) {
    if (!this._renderer.geometryInHash(model.gid)) {
      this._renderer.createBuffers(model.gid, model);
    }

    this._renderer.drawBuffers(model.gid);
  }
};

module.exports = p5;

},{"../core/core":54,"./p5.Geometry":101}],100:[function(_dereq_,module,exports){
/**
 * @module Lights, Camera
 * @submodule Material
 * @for p5
 * @requires core
 */

'use strict';

var p5 = _dereq_('../core/core');
//require('./p5.Texture');

/**
 * Normal material for geometry. You can view all
 * possible materials in this
 * <a href="https://p5js.org/examples/3d-materials.html">example</a>.
 * @method normalMaterial
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw(){
 *  background(200);
 *  normalMaterial();
 *  sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * Red, green and blue gradient.
 *
 */
p5.prototype.normalMaterial = function(){
  this._renderer._getShader('normalVert', 'normalFrag');
  return this;
};

/**
 * Texture for geometry.  You can view other possible materials in this
 * <a href="https://p5js.org/examples/3d-materials.html">example</a>.
 * @method texture
 * @param {p5.Image | p5.MediaElement | p5.Graphics} tex 2-dimensional graphics
 *                    to render as texture
 * @chainable
 * @example
 * <div>
 * <code>
 * var img;
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *   img = loadImage("assets/laDefense.jpg");
 * }
 *
 * function draw(){
 *   background(0);
 *   rotateZ(frameCount * 0.01);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   //pass image as texture
 *   texture(img);
 *   box(200, 200, 200);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var pg;
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 *   pg = createGraphics(200, 200);
 *   pg.textSize(100);
 * }
 *
 * function draw(){
 *   background(0);
 *   pg.background(255);
 *   pg.text('hello!', 0, 100);
 *   //pass image as texture
 *   texture(pg);
 *   plane(200);
 * }
 * </code>
 * </div>
 *
 * <div>
 * <code>
 * var vid;
 * function preload(){
 *   vid = createVideo("assets/fingers.mov");
 *   vid.hide();
 *   vid.loop();
 * }
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw(){
 *   background(0);
 *   //pass video frame as texture
 *   texture(vid);
 *   plane(200);
 * }
 * </code>
 * </div>
 *
 * @alt
 * Rotating view of many images umbrella and grid roof on a 3d plane
 * black canvas
 * black canvas
 *
 */
p5.prototype.texture = function(){
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  var gl = this._renderer.GL;
  gl.enable(gl.BLEND);
  gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
  this._renderer.drawMode = 'texture';
  var shaderProgram = this._renderer._getShader('lightVert',
    'lightTextureFrag');
  gl.useProgram(shaderProgram);
  var textureData;
  //if argument is not already a texture
  //create a new one
  if(!args[0].isTexture){
    if (args[0] instanceof p5.Image) {
      textureData = args[0].canvas;
    }
    //if param is a video
    else if (typeof p5.MediaElement !== 'undefined' &&
            args[0] instanceof p5.MediaElement){
      if(!args[0].loadedmetadata) {return;}
      textureData = args[0].elt;
    }
    //used with offscreen 2d graphics renderer
    else if(args[0] instanceof p5.Graphics){
      textureData = args[0].elt;
    }
    var tex = gl.createTexture();
    args[0]._setProperty('tex', tex);
    args[0]._setProperty('isTexture', true);
    this._renderer._bind.call(this, tex, textureData);
  }
  else {
    if(args[0] instanceof p5.Graphics ||
      (typeof p5.MediaElement !== 'undefined' &&
      args[0] instanceof p5.MediaElement)){
      textureData = args[0].elt;
    }
    else if(args[0] instanceof p5.Image){
      textureData = args[0].canvas;
    }
    this._renderer._bind.call(this, args[0].tex, textureData);
  }
  //this is where we'd activate multi textures
  //eg. gl.activeTexture(gl.TEXTURE0 + (unit || 0));
  //but for now we just have a single texture.
  //@TODO need to extend this functionality
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, args[0].tex);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, 'isTexture'), true);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, 'uSampler'), 0);
  return this;
};

/**
 * Texture Util functions
 */
p5.RendererGL.prototype._bind = function(tex, data){
  var gl = this._renderer.GL;
  gl.bindTexture(gl.TEXTURE_2D, tex);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texImage2D(gl.TEXTURE_2D, 0,
    gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, data);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texParameteri(gl.TEXTURE_2D,
  gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D,
  gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D,
  gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D,
  gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.bindTexture(gl.TEXTURE_2D, null);
};

/**
 * Checks whether val is a pot
 * more info on power of 2 here:
 * https://www.opengl.org/wiki/NPOT_Texture
 * @param  {Number}  value
 * @return {Boolean}
 */
// function _isPowerOf2 (value){
//   return (value & (value - 1)) === 0;
// }

/**
 * returns the next highest power of 2 value
 * @param  {Number} value [description]
 * @return {Number}       [description]
 */
// function _nextHighestPOT (value){
//   --value;
//   for (var i = 1; i < 32; i <<= 1) {
//     value = value | value >> i;
//   }
//   return value + 1;

/**
 * Ambient material for geometry with a given color. You can view all
 * possible materials in this
 * <a href="https://p5js.org/examples/3d-materials.html">example</a>.
 * @method  ambientMaterial
 * @param  {Number|Array|String|p5.Color} v1  gray value,
 * red or hue value (depending on the current color mode),
 * or color Array, or CSS color string
 * @param  {Number}            [v2] optional: green or saturation value
 * @param  {Number}            [v3] optional: blue or brightness value
 * @param  {Number}            [a]  optional: opacity
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *  background(0);
 *  ambientLight(100);
 *  pointLight(250, 250, 250, 100, 100, 0);
 *  ambientMaterial(250);
 *  sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * radiating light source from top right of canvas
 *
 */
p5.prototype.ambientMaterial = function(v1, v2, v3, a) {
  var gl = this._renderer.GL;
  var shaderProgram =
    this._renderer._getShader('lightVert', 'lightTextureFrag');

  gl.useProgram(shaderProgram);
  shaderProgram.uMaterialColor = gl.getUniformLocation(
    shaderProgram, 'uMaterialColor' );
  var colors = this._renderer._applyColorBlend.apply(this._renderer, arguments);

  gl.uniform4f(shaderProgram.uMaterialColor,
    colors[0], colors[1], colors[2], colors[3]);

  shaderProgram.uSpecular = gl.getUniformLocation(
    shaderProgram, 'uSpecular' );
  gl.uniform1i(shaderProgram.uSpecular, false);

  gl.uniform1i(gl.getUniformLocation(shaderProgram, 'isTexture'), false);

  return this;
};

/**
 * Specular material for geometry with a given color. You can view all
 * possible materials in this
 * <a href="https://p5js.org/examples/3d-materials.html">example</a>.
 * @method specularMaterial
 * @param  {Number|Array|String|p5.Color} v1  gray value,
 * red or hue value (depending on the current color mode),
 * or color Array, or CSS color string
 * @param  {Number}            [v2] optional: green or saturation value
 * @param  {Number}            [v3] optional: blue or brightness value
 * @param  {Number}            [a]  optional: opacity
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 * function draw(){
 *  background(0);
 *  ambientLight(100);
 *  pointLight(250, 250, 250, 100, 100, 0);
 *  specularMaterial(250);
 *  sphere(50);
 * }
 * </code>
 * </div>
 *
 * @alt
 * diffused radiating light source from top right of canvas
 *
 */
p5.prototype.specularMaterial = function(v1, v2, v3, a) {
  var gl = this._renderer.GL;
  var shaderProgram =
    this._renderer._getShader('lightVert', 'lightTextureFrag');
  gl.useProgram(shaderProgram);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, 'isTexture'), false);
  shaderProgram.uMaterialColor = gl.getUniformLocation(
    shaderProgram, 'uMaterialColor' );
  var colors = this._renderer._applyColorBlend.apply(this._renderer, arguments);
  gl.uniform4f(shaderProgram.uMaterialColor,
    colors[0], colors[1], colors[2], colors[3]);
  shaderProgram.uSpecular = gl.getUniformLocation(
    shaderProgram, 'uSpecular' );
  gl.uniform1i(shaderProgram.uSpecular, true);

  return this;
};

/**
 * @private blends colors according to color components.
 * If alpha value is less than 1, we need to enable blending
 * on our gl context.  Otherwise opaque objects need to a depthMask.
 * @param  {Number} v1 [description]
 * @param  {Number} v2 [description]
 * @param  {Number} v3 [description]
 * @param  {Number} a  [description]
 * @return {[Number]}  Normalized numbers array
 */
p5.RendererGL.prototype._applyColorBlend = function(v1,v2,v3,a){
  var gl = this.GL;
  var color = this._pInst.color.apply(
    this._pInst, arguments);
  var colors = color._array;
  if(colors[colors.length-1] < 1.0){
    gl.depthMask(false);
    gl.enable(gl.BLEND);
    gl.blendEquation( gl.FUNC_ADD );
    gl.blendFunc( gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA );
  } else {
    gl.depthMask(true);
    gl.disable(gl.BLEND);
  }
  return colors;
};

module.exports = p5;

},{"../core/core":54}],101:[function(_dereq_,module,exports){
//some of the functions are adjusted from Three.js(http://threejs.org)

'use strict';

var p5 = _dereq_('../core/core');

/**
 * p5 Geometry class
 * @class p5.Geometry
 * @constructor
 * @param  {function | Object} vertData callback function or Object
 *                     containing routine(s) for vertex data generation
 * @param  {Number} [detailX] number of vertices on horizontal surface
 * @param  {Number} [detailY] number of vertices on horizontal surface
 * @param {function} [callback] function to call upon object instantiation.
 *
 */
p5.Geometry = function
(detailX, detailY, callback){
  //an array containing every vertex
  //@type [p5.Vector]
  this.vertices = [];
  //an array containing 1 normal per vertex
  //@type [p5.Vector]
  //[p5.Vector, p5.Vector, p5.Vector,p5.Vector, p5.Vector, p5.Vector,...]
  this.vertexNormals = [];
  //an array containing each three vertex indices that form a face
  //[[0, 1, 2], [2, 1, 3], ...]
  this.faces = [];
  //a 2D array containing uvs for every vertex
  //[[0.0,0.0],[1.0,0.0], ...]
  this.uvs = [];
  this.detailX = (detailX !== undefined) ? detailX: 1;
  this.detailY = (detailY !== undefined) ? detailY: 1;
  if(callback instanceof Function){
    callback.call(this);
  }
  return this; // TODO: is this a constructor?
};

p5.Geometry.prototype.computeFaces = function(){
  var sliceCount = this.detailX + 1;
  var a, b, c, d;
  for (var i = 0; i < this.detailY; i++){
    for (var j = 0; j < this.detailX; j++){
      a = i * sliceCount + j;// + offset;
      b = i * sliceCount + j + 1;// + offset;
      c = (i + 1)* sliceCount + j + 1;// + offset;
      d = (i + 1)* sliceCount + j;// + offset;
      this.faces.push([a, b, d]);
      this.faces.push([d, b, c]);
    }
  }
  return this;
};

p5.Geometry.prototype._getFaceNormal = function(faceId,vertId){
  //This assumes that vA->vB->vC is a counter-clockwise ordering
  var face = this.faces[faceId];
  var vA = this.vertices[face[vertId%3]];
  var vB = this.vertices[face[(vertId+1)%3]];
  var vC = this.vertices[face[(vertId+2)%3]];
  var n = p5.Vector.cross(
    p5.Vector.sub(vB,vA),
    p5.Vector.sub(vC,vA));
  var sinAlpha = p5.Vector.mag(n) /
  (p5.Vector.mag(p5.Vector.sub(vB,vA))*
    p5.Vector.mag(p5.Vector.sub(vC,vA)));
  n = n.normalize();
  return n.mult(Math.asin(sinAlpha));
};
/**
 * computes smooth normals per vertex as an average of each
 * face.
 * @chainable
 */
p5.Geometry.prototype.computeNormals = function (){
  for(var v=0; v < this.vertices.length; v++){
    var normal = new p5.Vector();
    for(var i=0; i < this.faces.length; i++){
      //if our face contains a given vertex
      //calculate an average of the normals
      //of the triangles adjacent to that vertex
      if(this.faces[i][0] === v ||
        this.faces[i][1] === v ||
        this.faces[i][2] === v)
      {
        normal = normal.add(this._getFaceNormal(i, v));
      }
    }
    normal = normal.normalize();
    this.vertexNormals.push(normal);
  }
  return this;
};

/**
 * Averages the vertex normals. Used in curved
 * surfaces
 * @chainable
 */
p5.Geometry.prototype.averageNormals = function() {

  for(var i = 0; i <= this.detailY; i++){
    var offset = this.detailX + 1;
    var temp = p5.Vector
      .add(this.vertexNormals[i*offset],
        this.vertexNormals[i*offset + this.detailX]);
    temp = p5.Vector.div(temp, 2);
    this.vertexNormals[i*offset] = temp;
    this.vertexNormals[i*offset + this.detailX] = temp;
  }
  return this;
};

/**
 * Averages pole normals.  Used in spherical primitives
 * @chainable
 */
p5.Geometry.prototype.averagePoleNormals = function() {

  //average the north pole
  var sum = new p5.Vector(0, 0, 0);
  for(var i = 0; i < this.detailX; i++){
    sum.add(this.vertexNormals[i]);
  }
  sum = p5.Vector.div(sum, this.detailX);

  for(i = 0; i < this.detailX; i++){
    this.vertexNormals[i] = sum;
  }

  //average the south pole
  sum = new p5.Vector(0, 0, 0);
  for(i = this.vertices.length - 1;
    i > this.vertices.length - 1 - this.detailX; i--){
    sum.add(this.vertexNormals[i]);
  }
  sum = p5.Vector.div(sum, this.detailX);

  for(i = this.vertices.length - 1;
    i > this.vertices.length - 1 - this.detailX; i--){
    this.vertexNormals[i] = sum;
  }
  return this;
};

/**
 * Modifies all vertices to be centered within the range -100 to 100.
 * @chainable
 */
p5.Geometry.prototype.normalize = function() {
  if(this.vertices.length > 0) {
    // Find the corners of our bounding box
    var maxPosition = this.vertices[0].copy();
    var minPosition = this.vertices[0].copy();

    for(var i = 0; i < this.vertices.length; i++) {
      maxPosition.x = Math.max(maxPosition.x, this.vertices[i].x);
      minPosition.x = Math.min(minPosition.x, this.vertices[i].x);
      maxPosition.y = Math.max(maxPosition.y, this.vertices[i].y);
      minPosition.y = Math.min(minPosition.y, this.vertices[i].y);
      maxPosition.z = Math.max(maxPosition.z, this.vertices[i].z);
      minPosition.z = Math.min(minPosition.z, this.vertices[i].z);
    }

    var center = p5.Vector.lerp(maxPosition, minPosition, 0.5);
    var dist = p5.Vector.sub(maxPosition, minPosition);
    var longestDist = Math.max(Math.max(dist.x, dist.y), dist.z);
    var scale = 200 / longestDist;

    for(i = 0; i < this.vertices.length; i++) {
      this.vertices[i].sub(center);
      this.vertices[i].mult(scale);
    }
  }
  return this;
};

module.exports = p5.Geometry;

},{"../core/core":54}],102:[function(_dereq_,module,exports){
/**
* @requires constants
* @todo see methods below needing further implementation.
* future consideration: implement SIMD optimizations
* when browser compatibility becomes available
* https://developer.mozilla.org/en-US/docs/Web/JavaScript/
*   Reference/Global_Objects/SIMD
*/

'use strict';

var p5 = _dereq_('../core/core');
var polarGeometry = _dereq_('../math/polargeometry');
var constants = _dereq_('../core/constants');
var GLMAT_ARRAY_TYPE = (
    typeof Float32Array !== 'undefined') ?
  Float32Array : Array;

/**
 * A class to describe a 4x4 matrix
 * for model and view matrix manipulation in the p5js webgl renderer.
 * class p5.Matrix
 * @constructor
 * @param {Array} [mat4] array literal of our 4x4 matrix
 */
p5.Matrix = function() {
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  // This is default behavior when object
  // instantiated using createMatrix()
  // @todo implement createMatrix() in core/math.js
  if(args[0] instanceof p5) {
    // save reference to p5 if passed in
    this.p5 = args[0];
    if(args[1] === 'mat3'){
      this.mat3 = args[2] || new GLMAT_ARRAY_TYPE([
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
      ]);
    }
    else {
      this.mat4  = args[1] || new GLMAT_ARRAY_TYPE([
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      ]);
    }
    // default behavior when object
    // instantiated using new p5.Matrix()
  } else {
    if(args[0] === 'mat3'){
      this.mat3 = args[1] || new GLMAT_ARRAY_TYPE([
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
      ]);
    }
    else {
      this.mat4 = args[0] || new GLMAT_ARRAY_TYPE([
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      ]);
    }
  }
  return this;
};

/**
 * Sets the x, y, and z component of the vector using two or three separate
 * variables, the data from a p5.Matrix, or the values from a float array.
 *
 * @param {p5.Matrix|Float32Array|Array} [inMatrix] the input p5.Matrix or
 *                                     an Array of length 16
 * @chainable
 */
p5.Matrix.prototype.set = function (inMatrix) {
  if (inMatrix instanceof p5.Matrix) {
    this.mat4 = inMatrix.mat4;
    return this;
  }
  else if (inMatrix instanceof GLMAT_ARRAY_TYPE) {
    this.mat4 = inMatrix;
    return this;
  }
  return this;
};

/**
 * Gets a copy of the vector, returns a p5.Matrix object.
 *
 * @return {p5.Matrix} the copy of the p5.Matrix object
 */
p5.Matrix.prototype.get = function () {
  return new p5.Matrix(this.mat4);
};

/**
 * return a copy of a matrix
 * @return {p5.Matrix}   the result matrix
 */
p5.Matrix.prototype.copy = function(){
  var copied = new p5.Matrix();
  copied.mat4[0] = this.mat4[0];
  copied.mat4[1] = this.mat4[1];
  copied.mat4[2] = this.mat4[2];
  copied.mat4[3] = this.mat4[3];
  copied.mat4[4] = this.mat4[4];
  copied.mat4[5] = this.mat4[5];
  copied.mat4[6] = this.mat4[6];
  copied.mat4[7] = this.mat4[7];
  copied.mat4[8] = this.mat4[8];
  copied.mat4[9] = this.mat4[9];
  copied.mat4[10] = this.mat4[10];
  copied.mat4[11] = this.mat4[11];
  copied.mat4[12] = this.mat4[12];
  copied.mat4[13] = this.mat4[13];
  copied.mat4[14] = this.mat4[14];
  copied.mat4[15] = this.mat4[15];
  return copied;
};

/**
 * return an identity matrix
 * @return {p5.Matrix}   the result matrix
 */
p5.Matrix.identity = function(){
  return new p5.Matrix();
};

/**
 * transpose according to a given matrix
 * @param  {p5.Matrix|Float32Array|Array} a  the matrix to be based on to transpose
 * @chainable
 */
p5.Matrix.prototype.transpose = function(a){
  var a01, a02, a03, a12, a13, a23;
  if(a instanceof p5.Matrix){
    a01 = a.mat4[1];
    a02 = a.mat4[2];
    a03 = a.mat4[3];
    a12 = a.mat4[6];
    a13 = a.mat4[7];
    a23 = a.mat4[11];

    this.mat4[0] = a.mat4[0];
    this.mat4[1] = a.mat4[4];
    this.mat4[2] = a.mat4[8];
    this.mat4[3] = a.mat4[12];
    this.mat4[4] = a01;
    this.mat4[5] = a.mat4[5];
    this.mat4[6] = a.mat4[9];
    this.mat4[7] = a.mat4[13];
    this.mat4[8] = a02;
    this.mat4[9] = a12;
    this.mat4[10] = a.mat4[10];
    this.mat4[11] = a.mat4[14];
    this.mat4[12] = a03;
    this.mat4[13] = a13;
    this.mat4[14] = a23;
    this.mat4[15] = a.mat4[15];

  }else if(a instanceof GLMAT_ARRAY_TYPE){
    a01 = a[1];
    a02 = a[2];
    a03 = a[3];
    a12 = a[6];
    a13 = a[7];
    a23 = a[11];

    this.mat4[0] = a[0];
    this.mat4[1] = a[4];
    this.mat4[2] = a[8];
    this.mat4[3] = a[12];
    this.mat4[4] = a01;
    this.mat4[5] = a[5];
    this.mat4[6] = a[9];
    this.mat4[7] = a[13];
    this.mat4[8] = a02;
    this.mat4[9] = a12;
    this.mat4[10] = a[10];
    this.mat4[11] = a[14];
    this.mat4[12] = a03;
    this.mat4[13] = a13;
    this.mat4[14] = a23;
    this.mat4[15] = a[15];
  }
  return this;
};

/**
 * invert  matrix according to a give matrix
 * @param  {p5.Matrix|Float32Array|Array} a   the matrix to be based on to invert
 * @chainable
 */
p5.Matrix.prototype.invert = function(a){
  var a00, a01, a02, a03, a10, a11, a12, a13,
  a20, a21, a22, a23, a30, a31, a32, a33;
  if(a instanceof p5.Matrix){
    a00 = a.mat4[0];
    a01 = a.mat4[1];
    a02 = a.mat4[2];
    a03 = a.mat4[3];
    a10 = a.mat4[4];
    a11 = a.mat4[5];
    a12 = a.mat4[6];
    a13 = a.mat4[7];
    a20 = a.mat4[8];
    a21 = a.mat4[9];
    a22 = a.mat4[10];
    a23 = a.mat4[11];
    a30 = a.mat4[12];
    a31 = a.mat4[13];
    a32 = a.mat4[14];
    a33 = a.mat4[15];
  }else if(a instanceof GLMAT_ARRAY_TYPE){
    a00 = a[0];
    a01 = a[1];
    a02 = a[2];
    a03 = a[3];
    a10 = a[4];
    a11 = a[5];
    a12 = a[6];
    a13 = a[7];
    a20 = a[8];
    a21 = a[9];
    a22 = a[10];
    a23 = a[11];
    a30 = a[12];
    a31 = a[13];
    a32 = a[14];
    a33 = a[15];
  }
  var b00 = a00 * a11 - a01 * a10,
  b01 = a00 * a12 - a02 * a10,
  b02 = a00 * a13 - a03 * a10,
  b03 = a01 * a12 - a02 * a11,
  b04 = a01 * a13 - a03 * a11,
  b05 = a02 * a13 - a03 * a12,
  b06 = a20 * a31 - a21 * a30,
  b07 = a20 * a32 - a22 * a30,
  b08 = a20 * a33 - a23 * a30,
  b09 = a21 * a32 - a22 * a31,
  b10 = a21 * a33 - a23 * a31,
  b11 = a22 * a33 - a23 * a32,

  // Calculate the determinant
  det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 -
  b04 * b07 + b05 * b06;

  if (!det) {
    return null;
  }
  det = 1.0 / det;

  this.mat4[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
  this.mat4[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
  this.mat4[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
  this.mat4[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
  this.mat4[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
  this.mat4[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
  this.mat4[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
  this.mat4[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
  this.mat4[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
  this.mat4[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
  this.mat4[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
  this.mat4[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
  this.mat4[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
  this.mat4[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
  this.mat4[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
  this.mat4[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;

  return this;
};

/**
 * Inverts a 3x3 matrix
 * @chainable
 */
p5.Matrix.prototype.invert3x3 = function (){
  var a00 = this.mat3[0],
  a01 = this.mat3[1],
  a02 = this.mat3[2],
  a10 = this.mat3[3],
  a11 = this.mat3[4],
  a12 = this.mat3[5],
  a20 = this.mat3[6],
  a21 = this.mat3[7],
  a22 = this.mat3[8],
  b01 = a22 * a11 - a12 * a21,
  b11 = -a22 * a10 + a12 * a20,
  b21 = a21 * a10 - a11 * a20,

  // Calculate the determinant
  det = a00 * b01 + a01 * b11 + a02 * b21;
  if (!det) {
    return null;
  }
  det = 1.0 / det;
  this.mat3[0] = b01 * det;
  this.mat3[1] = (-a22 * a01 + a02 * a21) * det;
  this.mat3[2] = (a12 * a01 - a02 * a11) * det;
  this.mat3[3] = b11 * det;
  this.mat3[4] = (a22 * a00 - a02 * a20) * det;
  this.mat3[5] = (-a12 * a00 + a02 * a10) * det;
  this.mat3[6] = b21 * det;
  this.mat3[7] = (-a21 * a00 + a01 * a20) * det;
  this.mat3[8] = (a11 * a00 - a01 * a10) * det;
  return this;
};

/**
 * transposes a 3x3 p5.Matrix by a mat3
 * @param  {[Number]} mat3 1-dimensional array
 * @chainable
 */
p5.Matrix.prototype.transpose3x3 = function (mat3){
  var a01 = mat3[1], a02 = mat3[2], a12 = mat3[5];
  this.mat3[1] = mat3[3];
  this.mat3[2] = mat3[6];
  this.mat3[3] = a01;
  this.mat3[5] = mat3[7];
  this.mat3[6] = a02;
  this.mat3[7] = a12;
  return this;
};

/**
 * converts a 4x4 matrix to its 3x3 inverse tranform
 * commonly used in MVMatrix to NMatrix conversions.
 * @param  {p5.Matrix} mat4 the matrix to be based on to invert
 * @chainable
 * @todo  finish implementation
 */
p5.Matrix.prototype.inverseTranspose = function (matrix){
  if(this.mat3 === undefined){
    console.error('sorry, this function only works with mat3');
  }
  else {
    //convert mat4 -> mat3
    this.mat3[0] = matrix.mat4[0];
    this.mat3[1] = matrix.mat4[1];
    this.mat3[2] = matrix.mat4[2];
    this.mat3[3] = matrix.mat4[4];
    this.mat3[4] = matrix.mat4[5];
    this.mat3[5] = matrix.mat4[6];
    this.mat3[6] = matrix.mat4[8];
    this.mat3[7] = matrix.mat4[9];
    this.mat3[8] = matrix.mat4[10];
  }

  this.invert3x3().transpose3x3(this.mat3);
  return this;
};

/**
 * inspired by Toji's mat4 determinant
 * @return {Number} Determinant of our 4x4 matrix
 */
p5.Matrix.prototype.determinant = function(){
  var d00 = (this.mat4[0] * this.mat4[5]) - (this.mat4[1] * this.mat4[4]),
    d01 = (this.mat4[0] * this.mat4[6]) - (this.mat4[2] * this.mat4[4]),
    d02 = (this.mat4[0] * this.mat4[7]) - (this.mat4[3] * this.mat4[4]),
    d03 = (this.mat4[1] * this.mat4[6]) - (this.mat4[2] * this.mat4[5]),
    d04 = (this.mat4[1] * this.mat4[7]) - (this.mat4[3] * this.mat4[5]),
    d05 = (this.mat4[2] * this.mat4[7]) - (this.mat4[3] * this.mat4[6]),
    d06 = (this.mat4[8] * this.mat4[13]) - (this.mat4[9] * this.mat4[12]),
    d07 = (this.mat4[8] * this.mat4[14]) - (this.mat4[10] * this.mat4[12]),
    d08 = (this.mat4[8] * this.mat4[15]) - (this.mat4[11] * this.mat4[12]),
    d09 = (this.mat4[9] * this.mat4[14]) - (this.mat4[10] * this.mat4[13]),
    d10 = (this.mat4[9] * this.mat4[15]) - (this.mat4[11] * this.mat4[13]),
    d11 = (this.mat4[10] * this.mat4[15]) - (this.mat4[11] * this.mat4[14]);

  // Calculate the determinant
  return d00 * d11 - d01 * d10 + d02 * d09 +
    d03 * d08 - d04 * d07 + d05 * d06;
};

/**
 * multiply two mat4s
 * @param {p5.Matrix|Float32Array|Array} multMatrix The matrix
 *                                                we want to multiply by
 * @chainable
 */
p5.Matrix.prototype.mult = function(multMatrix){
  var _dest = new GLMAT_ARRAY_TYPE(16);
  var _src = new GLMAT_ARRAY_TYPE(16);

  if(multMatrix instanceof p5.Matrix) {
    _src = multMatrix.mat4;
  }
  else if(multMatrix instanceof GLMAT_ARRAY_TYPE){
    _src = multMatrix;
  }

  // each row is used for the multiplier
  var b0  = this.mat4[0], b1 = this.mat4[1],
    b2 = this.mat4[2], b3 = this.mat4[3];
  _dest[0] = b0*_src[0] + b1*_src[4] + b2*_src[8] + b3*_src[12];
  _dest[1] = b0*_src[1] + b1*_src[5] + b2*_src[9] + b3*_src[13];
  _dest[2] = b0*_src[2] + b1*_src[6] + b2*_src[10] + b3*_src[14];
  _dest[3] = b0*_src[3] + b1*_src[7] + b2*_src[11] + b3*_src[15];

  b0 = this.mat4[4];
  b1 = this.mat4[5];
  b2 = this.mat4[6];
  b3 = this.mat4[7];
  _dest[4] = b0*_src[0] + b1*_src[4] + b2*_src[8] + b3*_src[12];
  _dest[5] = b0*_src[1] + b1*_src[5] + b2*_src[9] + b3*_src[13];
  _dest[6] = b0*_src[2] + b1*_src[6] + b2*_src[10] + b3*_src[14];
  _dest[7] = b0*_src[3] + b1*_src[7] + b2*_src[11] + b3*_src[15];

  b0 = this.mat4[8];
  b1 = this.mat4[9];
  b2 = this.mat4[10];
  b3 = this.mat4[11];
  _dest[8] = b0*_src[0] + b1*_src[4] + b2*_src[8] + b3*_src[12];
  _dest[9] = b0*_src[1] + b1*_src[5] + b2*_src[9] + b3*_src[13];
  _dest[10] = b0*_src[2] + b1*_src[6] + b2*_src[10] + b3*_src[14];
  _dest[11] = b0*_src[3] + b1*_src[7] + b2*_src[11] + b3*_src[15];

  b0 = this.mat4[12];
  b1 = this.mat4[13];
  b2 = this.mat4[14];
  b3 = this.mat4[15];
  _dest[12] = b0*_src[0] + b1*_src[4] + b2*_src[8] + b3*_src[12];
  _dest[13] = b0*_src[1] + b1*_src[5] + b2*_src[9] + b3*_src[13];
  _dest[14] = b0*_src[2] + b1*_src[6] + b2*_src[10] + b3*_src[14];
  _dest[15] = b0*_src[3] + b1*_src[7] + b2*_src[11] + b3*_src[15];

  this.mat4 = _dest;

  return this;
};

/**
 * scales a p5.Matrix by scalars or a vector
 * @param  {p5.Vector|Float32Array|Array} s vector to scale by
 * @chainable
 */
p5.Matrix.prototype.scale = function() {
  var x,y,z;
  var args = new Array(arguments.length);
  for (var i = 0; i < args.length; ++i) {
    args[i] = arguments[i];
  }
  //if our 1st arg is a type p5.Vector
  if (args[0] instanceof p5.Vector){
    x = args[0].x;
    y = args[0].y;
    z = args[0].z;
  }
  //otherwise if it's an array
  else if (args[0] instanceof Array){
    x = args[0][0];
    y = args[0][1];
    z = args[0][2];
  }
  var _dest = new GLMAT_ARRAY_TYPE(16);
  _dest[0] = this.mat4[0] * x;
  _dest[1] = this.mat4[1] * x;
  _dest[2] = this.mat4[2] * x;
  _dest[3] = this.mat4[3] * x;
  _dest[4] = this.mat4[4] * y;
  _dest[5] = this.mat4[5] * y;
  _dest[6] = this.mat4[6] * y;
  _dest[7] = this.mat4[7] * y;
  _dest[8] = this.mat4[8] * z;
  _dest[9] = this.mat4[9] * z;
  _dest[10] = this.mat4[10] * z;
  _dest[11] = this.mat4[11] * z;
  _dest[12] = this.mat4[12];
  _dest[13] = this.mat4[13];
  _dest[14] = this.mat4[14];
  _dest[15] = this.mat4[15];

  this.mat4 = _dest;
  return this;
};

/**
 * rotate our Matrix around an axis by the given angle.
 * @param  {Number} a The angle of rotation in radians
 * @param  {p5.Vector|Array} axis  the axis(es) to rotate around
 * @chainable
 * inspired by Toji's gl-matrix lib, mat4 rotation
 */
p5.Matrix.prototype.rotate = function(a, axis){
  var x, y, z, _a, len;

  if (this.p5) {
    if (this.p5._angleMode === constants.DEGREES) {
      _a = polarGeometry.degreesToRadians(a);
    }
  }
  else {
    _a = a;
  }
  if (axis instanceof p5.Vector) {
    x = axis.x;
    y = axis.y;
    z = axis.z;
  }
  else if (axis instanceof Array) {
    x = axis[0];
    y = axis[1];
    z = axis[2];
  }

  len = Math.sqrt(x * x + y * y + z * z);
  x *= (1/len);
  y *= (1/len);
  z *= (1/len);

  var a00 = this.mat4[0];
  var a01 = this.mat4[1];
  var a02 = this.mat4[2];
  var a03 = this.mat4[3];
  var a10 = this.mat4[4];
  var a11 = this.mat4[5];
  var a12 = this.mat4[6];
  var a13 = this.mat4[7];
  var a20 = this.mat4[8];
  var a21 = this.mat4[9];
  var a22 = this.mat4[10];
  var a23 = this.mat4[11];

  //sin,cos, and tan of respective angle
  var sA = Math.sin(_a);
  var cA = Math.cos(_a);
  var tA = 1 - cA;
  // Construct the elements of the rotation matrix
  var b00 = x * x * tA + cA;
  var b01 = y * x * tA + z * sA;
  var b02 = z * x * tA - y * sA;
  var b10 = x * y * tA - z * sA;
  var b11 = y * y * tA + cA;
  var b12 = z * y * tA + x * sA;
  var b20 = x * z * tA + y * sA;
  var b21 = y * z * tA - x * sA;
  var b22 = z * z * tA + cA;

  // rotation-specific matrix multiplication
  this.mat4[0] = a00 * b00 + a10 * b01 + a20 * b02;
  this.mat4[1] = a01 * b00 + a11 * b01 + a21 * b02;
  this.mat4[2] = a02 * b00 + a12 * b01 + a22 * b02;
  this.mat4[3] = a03 * b00 + a13 * b01 + a23 * b02;
  this.mat4[4] = a00 * b10 + a10 * b11 + a20 * b12;
  this.mat4[5] = a01 * b10 + a11 * b11 + a21 * b12;
  this.mat4[6] = a02 * b10 + a12 * b11 + a22 * b12;
  this.mat4[7] = a03 * b10 + a13 * b11 + a23 * b12;
  this.mat4[8] = a00 * b20 + a10 * b21 + a20 * b22;
  this.mat4[9] = a01 * b20 + a11 * b21 + a21 * b22;
  this.mat4[10] = a02 * b20 + a12 * b21 + a22 * b22;
  this.mat4[11] = a03 * b20 + a13 * b21 + a23 * b22;

  return this;
};

/**
 * @todo  finish implementing this method!
 * translates
 * @param  {Number[]} v vector to translate by
 * @chainable
 */
p5.Matrix.prototype.translate = function(v){
  var x = v[0],
    y = v[1],
    z = v[2] || 0;
  this.mat4[12] =
    this.mat4[0] * x +this.mat4[4] * y +this.mat4[8] * z +this.mat4[12];
  this.mat4[13] =
    this.mat4[1] * x +this.mat4[5] * y +this.mat4[9] * z +this.mat4[13];
  this.mat4[14] =
    this.mat4[2] * x +this.mat4[6] * y +this.mat4[10] * z +this.mat4[14];
  this.mat4[15] =
    this.mat4[3] * x +this.mat4[7] * y +this.mat4[11] * z +this.mat4[15];
};

p5.Matrix.prototype.rotateX = function(a){
  this.rotate(a, [1,0,0]);
};
p5.Matrix.prototype.rotateY = function(a){
  this.rotate(a, [0,1,0]);
};
p5.Matrix.prototype.rotateZ = function(a){
  this.rotate(a, [0,0,1]);
};

/**
 * sets the perspective matrix
 * @param  {Number} fovy   [description]
 * @param  {Number} aspect [description]
 * @param  {Number} near   near clipping plane
 * @param  {Number} far    far clipping plane
 * @chainable
 */
p5.Matrix.prototype.perspective = function(fovy,aspect,near,far){

  var f = 1.0 / Math.tan(fovy / 2),
    nf = 1 / (near - far);

  this.mat4[0] = f / aspect;
  this.mat4[1] = 0;
  this.mat4[2] = 0;
  this.mat4[3] = 0;
  this.mat4[4] = 0;
  this.mat4[5] = f;
  this.mat4[6] = 0;
  this.mat4[7] = 0;
  this.mat4[8] = 0;
  this.mat4[9] = 0;
  this.mat4[10] = (far + near) * nf;
  this.mat4[11] = -1;
  this.mat4[12] = 0;
  this.mat4[13] = 0;
  this.mat4[14] = (2 * far * near) * nf;
  this.mat4[15] = 0;

  return this;

};

/**
 * sets the ortho matrix
 * @param  {Number} left   [description]
 * @param  {Number} right  [description]
 * @param  {Number} bottom [description]
 * @param  {Number} top    [description]
 * @param  {Number} near   near clipping plane
 * @param  {Number} far    far clipping plane
 * @chainable
 */
p5.Matrix.prototype.ortho = function(left,right,bottom,top,near,far){

  var lr = 1 / (left - right),
    bt = 1 / (bottom - top),
    nf = 1 / (near - far);
  this.mat4[0] = -2 * lr;
  this.mat4[1] = 0;
  this.mat4[2] = 0;
  this.mat4[3] = 0;
  this.mat4[4] = 0;
  this.mat4[5] = -2 * bt;
  this.mat4[6] = 0;
  this.mat4[7] = 0;
  this.mat4[8] = 0;
  this.mat4[9] = 0;
  this.mat4[10] = 2 * nf;
  this.mat4[11] = 0;
  this.mat4[12] = (left + right) * lr;
  this.mat4[13] = (top + bottom) * bt;
  this.mat4[14] = (far + near) * nf;
  this.mat4[15] = 1;

  return this;
};

/**
 * PRIVATE
 */
// matrix methods adapted from:
// https://developer.mozilla.org/en-US/docs/Web/WebGL/
// gluPerspective
//
// function _makePerspective(fovy, aspect, znear, zfar){
//    var ymax = znear * Math.tan(fovy * Math.PI / 360.0);
//    var ymin = -ymax;
//    var xmin = ymin * aspect;
//    var xmax = ymax * aspect;
//    return _makeFrustum(xmin, xmax, ymin, ymax, znear, zfar);
//  }

////
//// glFrustum
////
//function _makeFrustum(left, right, bottom, top, znear, zfar){
//  var X = 2*znear/(right-left);
//  var Y = 2*znear/(top-bottom);
//  var A = (right+left)/(right-left);
//  var B = (top+bottom)/(top-bottom);
//  var C = -(zfar+znear)/(zfar-znear);
//  var D = -2*zfar*znear/(zfar-znear);
//  var frustrumMatrix =[
//  X, 0, A, 0,
//  0, Y, B, 0,
//  0, 0, C, D,
//  0, 0, -1, 0
//];
//return frustrumMatrix;
// }

// function _setMVPMatrices(){
////an identity matrix
////@TODO use the p5.Matrix class to abstract away our MV matrices and
///other math
//var _mvMatrix =
//[
//  1.0,0.0,0.0,0.0,
//  0.0,1.0,0.0,0.0,
//  0.0,0.0,1.0,0.0,
//  0.0,0.0,0.0,1.0
//];

module.exports = p5.Matrix;

},{"../core/constants":53,"../core/core":54,"../math/polargeometry":86}],103:[function(_dereq_,module,exports){
/**
 * Welcome to RendererGL Immediate Mode.
 * Immediate mode is used for drawing custom shapes
 * from a set of vertices.  Immediate Mode is activated
 * when you call beginShape() & de-activated when you call endShape().
 * Immediate mode is a style of programming borrowed
 * from OpenGL's (now-deprecated) immediate mode.
 * It differs from p5.js' default, Retained Mode, which caches
 * geometries and buffers on the CPU to reduce the number of webgl
 * draw calls. Retained mode is more efficient & performative,
 * however, Immediate Mode is useful for sketching quick
 * geometric ideas.
 */
'use strict';

var p5 = _dereq_('../core/core');
var constants = _dereq_('../core/constants');

/**
 * Begin shape drawing.  This is a helpful way of generating
 * custom shapes quickly.  However in WEBGL mode, application
 * performance will likely drop as a result of too many calls to
 * beginShape() / endShape().  As a high performance alternative,
 * please use p5.js geometry primitives.
 * @param  {Number} mode webgl primitives mode.  beginShape supports the
 *                       following modes:
 *                       POINTS,LINES,LINE_STRIP,LINE_LOOP,TRIANGLES,
 *                       TRIANGLE_STRIP,and TRIANGLE_FAN.
 * @chainable
 */
p5.RendererGL.prototype.beginShape = function(mode){
  //default shape mode is line_strip
  this.immediateMode.shapeMode = (mode !== undefined ) ?
    mode : constants.LINE_STRIP;
  //if we haven't yet initialized our
  //immediateMode vertices & buffers, create them now!
  if(this.immediateMode.vertexPositions === undefined){
    this.immediateMode.vertexPositions = [];
    this.immediateMode.vertexColors = [];
    this.immediateMode.vertexBuffer = this.GL.createBuffer();
    this.immediateMode.colorBuffer = this.GL.createBuffer();
  } else {
    this.immediateMode.vertexPositions.length = 0;
    this.immediateMode.vertexColors.length = 0;
  }
  this.isImmediateDrawing = true;
  return this;
};
/**
 * adds a vertex to be drawn in a custom Shape.
 * @param  {Number} x x-coordinate of vertex
 * @param  {Number} y y-coordinate of vertex
 * @param  {Number} z z-coordinate of vertex
 * @chainable
 * @TODO implement handling of p5.Vector args
 */
p5.RendererGL.prototype.vertex = function(x, y, z){
  this.immediateMode.vertexPositions.push(x, y, z);
  var vertexColor = this.curFillColor || [0.5, 0.5, 0.5, 1.0];
  this.immediateMode.vertexColors.push(
    vertexColor[0],
    vertexColor[1],
    vertexColor[2],
    vertexColor[3]);
  return this;
};

/**
 * End shape drawing and render vertices to screen.
 * @chainable
 */
p5.RendererGL.prototype.endShape =
function(mode, isCurve, isBezier,isQuadratic, isContour, shapeKind){
  var gl = this.GL;
  this._bindImmediateBuffers(
    this.immediateMode.vertexPositions,
    this.immediateMode.vertexColors);
  if(mode){
    if(this.drawMode === 'fill' || this.drawMode ==='texture'){
      switch(this.immediateMode.shapeMode){
        case constants.LINE_STRIP:
          this.immediateMode.shapeMode = constants.TRIANGLE_FAN;
          break;
        case constants.LINES:
          this.immediateMode.shapeMode = constants.TRIANGLE_FAN;
          break;
        case constants.TRIANGLES:
          this.immediateMode.shapeMode = constants.TRIANGLE_FAN;
          break;
      }
    } else {
      switch(this.immediateMode.shapeMode){
        case constants.LINE_STRIP:
          this.immediateMode.shapeMode = constants.LINE_LOOP;
          break;
        case constants.LINES:
          this.immediateMode.shapeMode = constants.LINE_LOOP;
          break;
      }
    }
  }
  //QUADS & QUAD_STRIP are not supported primitives modes
  //in webgl.
  if(this.immediateMode.shapeMode === constants.QUADS ||
    this.immediateMode.shapeMode === constants.QUAD_STRIP){
    throw new Error('sorry, ' + this.immediateMode.shapeMode+
      ' not yet implemented in webgl mode.');
  }
  else {
    gl.enable(gl.BLEND);
    gl.drawArrays(this.immediateMode.shapeMode, 0,
      this.immediateMode.vertexPositions.length / 3);
  }
  //clear out our vertexPositions & colors arrays
  //after rendering
  this.immediateMode.vertexPositions.length = 0;
  this.immediateMode.vertexColors.length = 0;
  this.isImmediateDrawing = false;
  return this;
};
/**
 * Bind immediateMode buffers to data,
 * then draw gl arrays
 * @param  {Number[]} vertices Numbers array representing
 *                          vertex positions
 * @chainable
 */
p5.RendererGL.prototype._bindImmediateBuffers = function(vertices, colors){
  this._setDefaultCamera();
  var gl = this.GL;
  var shaderKey = this._getCurShaderId();
  var shaderProgram = this.mHash[shaderKey];
  //vertex position Attribute
  shaderProgram.vertexPositionAttribute =
    gl.getAttribLocation(shaderProgram, 'aPosition');
  gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);
  gl.bindBuffer(gl.ARRAY_BUFFER, this.immediateMode.vertexBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER, new Float32Array(vertices), gl.DYNAMIC_DRAW);
  gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute,
    3, gl.FLOAT, false, 0, 0);

  shaderProgram.vertexColorAttribute =
    gl.getAttribLocation(shaderProgram, 'aVertexColor');
  if (shaderProgram.vertexColorAttribute != -1) {
    gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.immediateMode.colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
      new Float32Array(colors),gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(shaderProgram.vertexColorAttribute,
      4, gl.FLOAT, false, 0, 0);
  }
  //matrix
  this._setMatrixUniforms(shaderKey);
  //@todo implement in all shaders (not just immediateVert)
  //set our default point size
  // this._setUniform1f(shaderKey,
  //   'uPointSize',
  //   this.pointSize);
  return this;
};

//////////////////////////////////////////////
// COLOR
//////////////////////////////////////////////

p5.RendererGL.prototype._getColorVertexShader = function(){
  var gl = this.GL;
  var mId = 'immediateVert|vertexColorFrag';
  var shaderProgram;

  if(!this.materialInHash(mId)){
    shaderProgram =
      this._initShaders('immediateVert', 'vertexColorFrag', true);
    this.mHash[mId] = shaderProgram;
    shaderProgram.vertexColorAttribute =
    gl.getAttribLocation(shaderProgram, 'aVertexColor');
    gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);
  }else{
    shaderProgram = this.mHash[mId];
  }
  return shaderProgram;
};

module.exports = p5.RendererGL;
},{"../core/constants":53,"../core/core":54}],104:[function(_dereq_,module,exports){
//Retained Mode. The default mode for rendering 3D primitives
//in WEBGL.
'use strict';

var p5 = _dereq_('../core/core');
var hashCount = 0;
/**
 * _initBufferDefaults
 * @description initializes buffer defaults. runs each time a new geometry is
 * registered
 * @param  {String} gId  key of the geometry object
 */
p5.RendererGL.prototype._initBufferDefaults = function(gId) {
  //@TODO remove this limit on hashes in gHash
  hashCount ++;
  if(hashCount > 1000){
    var key = Object.keys(this.gHash)[0];
    delete this.gHash[key];
    hashCount --;
  }

  var gl = this.GL;
  //create a new entry in our gHash
  this.gHash[gId] = {};
  this.gHash[gId].vertexBuffer = gl.createBuffer();
  this.gHash[gId].normalBuffer = gl.createBuffer();
  this.gHash[gId].uvBuffer = gl.createBuffer();
  this.gHash[gId].indexBuffer = gl.createBuffer();
};
/**
 * createBuffers description
 * @param  {String} gId    key of the geometry object
 * @param  {p5.Geometry}  obj contains geometry data
 */
p5.RendererGL.prototype.createBuffers = function(gId, obj) {
  var gl = this.GL;
  this._setDefaultCamera();
  //initialize the gl buffers for our geom groups
  this._initBufferDefaults(gId);
  //return the current shaderProgram from our material hash
  var shaderProgram = this.mHash[this._getCurShaderId()];
  //@todo rename "numberOfItems" property to something more descriptive
  //we mult the num geom faces by 3
  this.gHash[gId].numberOfItems = obj.faces.length * 3;
  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].vertexBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array( vToNArray(obj.vertices) ),
    gl.STATIC_DRAW);
  //vertex position
  shaderProgram.vertexPositionAttribute =
    gl.getAttribLocation(shaderProgram, 'aPosition');
  gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

  gl.vertexAttribPointer(
    shaderProgram.vertexPositionAttribute,
    3, gl.FLOAT, false, 0, 0);

  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].normalBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array( vToNArray(obj.vertexNormals) ),
    gl.STATIC_DRAW);
  //vertex normal
  shaderProgram.vertexNormalAttribute =
    gl.getAttribLocation(shaderProgram, 'aNormal');
  gl.enableVertexAttribArray(shaderProgram.vertexNormalAttribute);

  gl.vertexAttribPointer(
    shaderProgram.vertexNormalAttribute,
    3, gl.FLOAT, false, 0, 0);

  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].uvBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array( flatten(obj.uvs) ),
    gl.STATIC_DRAW);
  //texture coordinate Attribute
  shaderProgram.textureCoordAttribute =
    gl.getAttribLocation(shaderProgram, 'aTexCoord');
  gl.enableVertexAttribArray(shaderProgram.textureCoordAttribute);
  gl.vertexAttribPointer(
    shaderProgram.textureCoordAttribute,
    2, gl.FLOAT, false, 0, 0);

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.gHash[gId].indexBuffer);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint16Array( flatten(obj.faces) ),
    gl.STATIC_DRAW);
};

/**
 * Draws buffers given a geometry key ID
 * @param  {String} gId     ID in our geom hash
 * @chainable
 */
p5.RendererGL.prototype.drawBuffers = function(gId) {
  this._setDefaultCamera();
  var gl = this.GL;
  var shaderKey = this._getCurShaderId();
  var shaderProgram = this.mHash[shaderKey];
  //vertex position buffer
  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].vertexBuffer);
  gl.vertexAttribPointer(
    shaderProgram.vertexPositionAttribute,
    3, gl.FLOAT, false, 0, 0);
  //normal buffer
  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].normalBuffer);
  gl.vertexAttribPointer(
    shaderProgram.vertexNormalAttribute,
    3, gl.FLOAT, false, 0, 0);
  // uv buffer
  gl.bindBuffer(gl.ARRAY_BUFFER, this.gHash[gId].uvBuffer);
  gl.vertexAttribPointer(
    shaderProgram.textureCoordAttribute,
    2, gl.FLOAT, false, 0, 0);
  //vertex index buffer
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.gHash[gId].indexBuffer);
  this._setMatrixUniforms(shaderKey);
  gl.drawElements(
    gl.TRIANGLES, this.gHash[gId].numberOfItems,
    gl.UNSIGNED_SHORT, 0);
  return this;
};
///////////////////////////////
//// UTILITY FUNCTIONS
//////////////////////////////
/**
 * turn a two dimensional array into one dimensional array
 * @param  {Array} arr 2-dimensional array
 * @return {Array}     1-dimensional array
 * [[1, 2, 3],[4, 5, 6]] -> [1, 2, 3, 4, 5, 6]
 */
function flatten(arr){
  if (arr.length>0){
    return arr.reduce(function(a, b){
      return a.concat(b);
    });
  } else {
    return [];
  }
}

/**
 * turn a p5.Vector Array into a one dimensional number array
 * @param  {p5.Vector[]} arr  an array of p5.Vector
 * @return {Number[]}         a one dimensional array of numbers
 * [p5.Vector(1, 2, 3), p5.Vector(4, 5, 6)] ->
 * [1, 2, 3, 4, 5, 6]
 */
function vToNArray(arr){
  return flatten(arr.map(function(item){
    return [item.x, item.y, item.z];
  }));
}
module.exports = p5.RendererGL;

},{"../core/core":54}],105:[function(_dereq_,module,exports){
'use strict';

var p5 = _dereq_('../core/core');
var shader = _dereq_('./shader');
_dereq_('../core/p5.Renderer');
_dereq_('./p5.Matrix');
var uMVMatrixStack = [];

//@TODO should implement public method
//to override these attributes
var attributes = {
  alpha: true,
  depth: true,
  stencil: true,
  antialias: false,
  premultipliedAlpha: false,
  preserveDrawingBuffer: false
};

/**
 * 3D graphics class
 * @class p5.RendererGL
 * @constructor
 * @extends p5.Renderer
 * @todo extend class to include public method for offscreen
 * rendering (FBO).
 *
 */
p5.RendererGL = function(elt, pInst, isMainCanvas) {
  p5.Renderer.call(this, elt, pInst, isMainCanvas);
  this._initContext();

  this.isP3D = true; //lets us know we're in 3d mode
  this.GL = this.drawingContext;
  //lights
  this.ambientLightCount = 0;
  this.directionalLightCount = 0;
  this.pointLightCount = 0;
  //camera
  this._curCamera = null;

  /**
   * model view, projection, & normal
   * matrices
   */
  this.uMVMatrix = new p5.Matrix();
  this.uPMatrix  = new p5.Matrix();
  this.uNMatrix = new p5.Matrix('mat3');
  //Geometry & Material hashes
  this.gHash = {};
  this.mHash = {};
  //Imediate Mode
  //default drawing is done in Retained Mode
  this.isImmediateDrawing = false;
  this.immediateMode = {};
  this.curFillColor = [0.5,0.5,0.5,1.0];
  this.curStrokeColor = [0.5,0.5,0.5,1.0];
  this.pointSize = 5.0;//default point/stroke
  return this;
};

p5.RendererGL.prototype = Object.create(p5.Renderer.prototype);

//////////////////////////////////////////////
// Setting
//////////////////////////////////////////////

p5.RendererGL.prototype._initContext = function() {
  try {
    this.drawingContext = this.canvas.getContext('webgl', attributes) ||
      this.canvas.getContext('experimental-webgl', attributes);
    if (this.drawingContext === null) {
      throw new Error('Error creating webgl context');
    } else {
      console.log('p5.RendererGL: enabled webgl context');
      var gl = this.drawingContext;
      gl.enable(gl.DEPTH_TEST);
      gl.depthFunc(gl.LEQUAL);
      gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
    }
  } catch (er) {
    throw new Error(er);
  }
};
//detect if user didn't set the camera
//then call this function below
p5.RendererGL.prototype._setDefaultCamera = function(){
  if(this._curCamera === null){
    var _w = this.width;
    var _h = this.height;
    this.uPMatrix = p5.Matrix.identity();
    var cameraZ = (this.height / 2) / Math.tan(Math.PI * 30 / 180);
    this.uPMatrix.perspective(60 / 180 * Math.PI, _w / _h,
                              cameraZ * 0.1, cameraZ * 10);
    this._curCamera = 'default';
  }
};

p5.RendererGL.prototype._update = function() {
  this.uMVMatrix = p5.Matrix.identity();
  this.translate(0, 0, -(this.height / 2) / Math.tan(Math.PI * 30 / 180));
  this.ambientLightCount = 0;
  this.directionalLightCount = 0;
  this.pointLightCount = 0;
};

/**
 * [background description]
 */
p5.RendererGL.prototype.background = function() {
  var gl = this.GL;
  var _col = this._pInst.color.apply(this._pInst, arguments);
  var _r = (_col.levels[0]) / 255;
  var _g = (_col.levels[1]) / 255;
  var _b = (_col.levels[2]) / 255;
  var _a = (_col.levels[3]) / 255;
  gl.clearColor(_r, _g, _b, _a);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
};

//@TODO implement this
// p5.RendererGL.prototype.clear = function() {
//@TODO
// };

//////////////////////////////////////////////
// SHADER
//////////////////////////////////////////////

/**
 * [_initShaders description]
 * @param  {string} vertId [description]
 * @param  {string} fragId [description]
 * @return {Object} the shader program
 */
p5.RendererGL.prototype._initShaders =
function(vertId, fragId, isImmediateMode) {
  var gl = this.GL;
  //set up our default shaders by:
  // 1. create the shader,
  // 2. load the shader source,
  // 3. compile the shader
  var _vertShader = gl.createShader(gl.VERTEX_SHADER);
  //load in our default vertex shader
  gl.shaderSource(_vertShader, shader[vertId]);
  gl.compileShader(_vertShader);
  // if our vertex shader failed compilation?
  if (!gl.getShaderParameter(_vertShader, gl.COMPILE_STATUS)) {
    alert('Yikes! An error occurred compiling the shaders:' +
      gl.getShaderInfoLog(_vertShader));
    return null;
  }

  var _fragShader = gl.createShader(gl.FRAGMENT_SHADER);
  //load in our material frag shader
  gl.shaderSource(_fragShader, shader[fragId]);
  gl.compileShader(_fragShader);
  // if our frag shader failed compilation?
  if (!gl.getShaderParameter(_fragShader, gl.COMPILE_STATUS)) {
    alert('Darn! An error occurred compiling the shaders:' +
      gl.getShaderInfoLog(_fragShader));
    return null;
  }

  var shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, _vertShader);
  gl.attachShader(shaderProgram, _fragShader);
  gl.linkProgram(shaderProgram);
  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
    alert('Snap! Error linking shader program');
  }
  //END SHADERS SETUP

  this._getLocation(shaderProgram, isImmediateMode);

  return shaderProgram;
};

p5.RendererGL.prototype._getLocation =
function(shaderProgram, isImmediateMode) {
  var gl = this.GL;
  gl.useProgram(shaderProgram);

  //projection Matrix uniform
  shaderProgram.uPMatrixUniform =
    gl.getUniformLocation(shaderProgram, 'uProjectionMatrix');
  //model view Matrix uniform
  shaderProgram.uMVMatrixUniform =
    gl.getUniformLocation(shaderProgram, 'uModelViewMatrix');

  //@TODO: figure out a better way instead of if statement
  if(isImmediateMode === undefined){
    //normal Matrix uniform
    shaderProgram.uNMatrixUniform =
    gl.getUniformLocation(shaderProgram, 'uNormalMatrix');

    shaderProgram.samplerUniform =
    gl.getUniformLocation(shaderProgram, 'uSampler');
  }
};

/**
 * Sets a shader uniform given a shaderProgram and uniform string
 * @param {String} shaderKey key to material Hash.
 * @param {String} uniform location in shader.
 * @param {Number} data data to bind uniform.  Float data type.
 * @chainable
 * @todo currently this function sets uniform1f data.
 * Should generalize function to accept any uniform
 * data type.
 */
p5.RendererGL.prototype._setUniform1f = function(shaderKey,uniform,data)
{
  var gl = this.GL;
  var shaderProgram = this.mHash[shaderKey];
  gl.useProgram(shaderProgram);
  shaderProgram[uniform] = gl.getUniformLocation(shaderProgram, uniform);
  gl.uniform1f(shaderProgram[uniform], data);
  return this;
};

p5.RendererGL.prototype._setMatrixUniforms = function(shaderKey) {
  var gl = this.GL;
  var shaderProgram = this.mHash[shaderKey];

  gl.useProgram(shaderProgram);

  gl.uniformMatrix4fv(
    shaderProgram.uPMatrixUniform,
    false, this.uPMatrix.mat4);

  gl.uniformMatrix4fv(
    shaderProgram.uMVMatrixUniform,
    false, this.uMVMatrix.mat4);

  this.uNMatrix.inverseTranspose(this.uMVMatrix);

  gl.uniformMatrix3fv(
    shaderProgram.uNMatrixUniform,
    false, this.uNMatrix.mat3);
};
//////////////////////////////////////////////
// GET CURRENT | for shader and color
//////////////////////////////////////////////
p5.RendererGL.prototype._getShader = function(vertId, fragId, isImmediateMode) {
  var mId = vertId + '|' + fragId;
  //create it and put it into hashTable
  if(!this.materialInHash(mId)){
    var shaderProgram = this._initShaders(vertId, fragId, isImmediateMode);
    this.mHash[mId] = shaderProgram;
  }
  this.curShaderId = mId;

  return this.mHash[this.curShaderId];
};

p5.RendererGL.prototype._getCurShaderId = function(){
  //if the shader ID is not yet defined
  if(this.drawMode !== 'fill' && this.curShaderId === undefined){
    //default shader: normalMaterial()
    var mId = 'normalVert|normalFrag';
    var shaderProgram = this._initShaders('normalVert', 'normalFrag');
    this.mHash[mId] = shaderProgram;
    this.curShaderId = mId;
  } else if(this.isImmediateDrawing && this.drawMode === 'fill'){
    // note that this._getShader will check if the shader already exists
    // by looking up the shader id (composed of vertexShaderId|fragmentShaderId)
    // in the material hash. If the material isn't found in the hash, it
    // creates a new one using this._initShaders--however, we'd like
    // use the cached version as often as possible, so we defer to this._getShader
    // here instead of calling this._initShaders directly.
    this._getShader('immediateVert', 'vertexColorFrag', true);
  }

  return this.curShaderId;
};

//////////////////////////////////////////////
// COLOR
//////////////////////////////////////////////
/**
 * Basic fill material for geometry with a given color
 * @method  fill
 * @param  {Number|Array|String|p5.Color} v1  gray value,
 * red or hue value (depending on the current color mode),
 * or color Array, or CSS color string
 * @param  {Number}            [v2] optional: green or saturation value
 * @param  {Number}            [v3] optional: blue or brightness value
 * @param  {Number}            [a]  optional: opacity
 * @chainable
 * @example
 * <div>
 * <code>
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw(){
 *  background(0);
 *  fill(250, 0, 0);
 *  rotateX(frameCount * 0.01);
 *  rotateY(frameCount * 0.01);
 *  rotateZ(frameCount * 0.01);
 *  box(200, 200, 200);
 * }
 * </code>
 * </div>
 *
 * @alt
 * red canvas
 *
 */
p5.RendererGL.prototype.fill = function(v1, v2, v3, a) {
  var gl = this.GL;
  var shaderProgram;
  //see material.js for more info on color blending in webgl
  var colors = this._applyColorBlend.apply(this, arguments);
  this.curFillColor = colors;
  this.drawMode = 'fill';
  if(this.isImmediateDrawing){
    shaderProgram =
    this._getShader('immediateVert','vertexColorFrag');
    gl.useProgram(shaderProgram);
  } else {
    shaderProgram =
    this._getShader('normalVert', 'basicFrag');
    gl.useProgram(shaderProgram);
    //RetainedMode uses a webgl uniform to pass color vals
    //in ImmediateMode, we want access to each vertex so therefore
    //we cannot use a uniform.
    shaderProgram.uMaterialColor = gl.getUniformLocation(
      shaderProgram, 'uMaterialColor' );
    gl.uniform4f( shaderProgram.uMaterialColor,
      colors[0],
      colors[1],
      colors[2],
      colors[3]);
  }
  return this;
};
p5.RendererGL.prototype.stroke = function(r, g, b, a) {
  var color = this._pInst.color.apply(this._pInst, arguments);
  var colorNormalized = color._array;
  this.curStrokeColor = colorNormalized;
  this.drawMode = 'stroke';
  return this;
};

//@TODO
p5.RendererGL.prototype._strokeCheck = function(){
  if(this.drawMode === 'stroke'){
    throw new Error(
      'stroke for shapes in 3D not yet implemented, use fill for now :('
    );
  }
};

/**
 * [strokeWeight description]
 * @param  {Number} pointSize stroke point size
 * @chainable
 * @todo  strokeWeight currently works on points only.
 * implement on all wireframes and strokes.
 */
p5.RendererGL.prototype.strokeWeight = function(pointSize) {
  this.pointSize = pointSize;
  return this;
};
//////////////////////////////////////////////
// HASH | for material and geometry
//////////////////////////////////////////////

p5.RendererGL.prototype.geometryInHash = function(gId){
  return this.gHash[gId] !== undefined;
};

p5.RendererGL.prototype.materialInHash = function(mId){
  return this.mHash[mId] !== undefined;
};

/**
 * [resize description]
 * @param  {Number} w [description]
 * @param  {Number} h [description]
 */
p5.RendererGL.prototype.resize = function(w,h) {
  var gl = this.GL;
  p5.Renderer.prototype.resize.call(this, w, h);
  gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
  // If we're using the default camera, update the aspect ratio
  if(this._curCamera === 'default') {
    this._curCamera = null;
    this._setDefaultCamera();
  }
};

/**
 * clears color and depth buffers
 * with r,g,b,a
 * @param {Number} r normalized red val.
 * @param {Number} g normalized green val.
 * @param {Number} b normalized blue val.
 * @param {Number} a normalized alpha val.
 */
p5.RendererGL.prototype.clear = function() {
  var gl = this.GL;
  gl.clearColor(arguments[0],
    arguments[1],
    arguments[2],
    arguments[3]);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
};

/**
 * [translate description]
 * @param  {Number} x [description]
 * @param  {Number} y [description]
 * @param  {Number} z [description]
 * @chainable
 * @todo implement handle for components or vector as args
 */
p5.RendererGL.prototype.translate = function(x, y, z) {
  this.uMVMatrix.translate([x,-y,z]);
  return this;
};

/**
 * Scales the Model View Matrix by a vector
 * @param  {Number | p5.Vector | Array} x [description]
 * @param  {Number} [y] y-axis scalar
 * @param  {Number} [z] z-axis scalar
 * @chainable
 */
p5.RendererGL.prototype.scale = function(x,y,z) {
  this.uMVMatrix.scale([x,y,z]);
  return this;
};

p5.RendererGL.prototype.rotate = function(rad, axis){
  this.uMVMatrix.rotate(rad, axis);
  return this;
};

p5.RendererGL.prototype.rotateX = function(rad) {
  this.rotate(rad, [1,0,0]);
  return this;
};

p5.RendererGL.prototype.rotateY = function(rad) {
  this.rotate(rad, [0,1,0]);
  return this;
};

p5.RendererGL.prototype.rotateZ = function(rad) {
  this.rotate(rad, [0,0,1]);
  return this;
};

/**
 * pushes a copy of the model view matrix onto the
 * MV Matrix stack.
 */
p5.RendererGL.prototype.push = function() {
  uMVMatrixStack.push(this.uMVMatrix.copy());
};

/**
 * [pop description]
 */
p5.RendererGL.prototype.pop = function() {
  if (uMVMatrixStack.length === 0) {
    throw new Error('Invalid popMatrix!');
  }
  this.uMVMatrix = uMVMatrixStack.pop();
};

p5.RendererGL.prototype.resetMatrix = function() {
  this.uMVMatrix = p5.Matrix.identity();
  this.translate(0, 0, -800);
  return this;
};

// Text/Typography
// @TODO:
p5.RendererGL.prototype._applyTextProperties = function() {
  //@TODO finish implementation
  console.error('text commands not yet implemented in webgl');
};
module.exports = p5.RendererGL;

},{"../core/core":54,"../core/p5.Renderer":61,"./p5.Matrix":102,"./shader":107}],106:[function(_dereq_,module,exports){
/**
 * @module Shape
 * @submodule 3D Primitives
 * @for p5
 * @requires core
 * @requires p5.Geometry
 */

'use strict';

var p5 = _dereq_('../core/core');
_dereq_('./p5.Geometry');

/**
 * Draw a line given two points
 * @param {Number} x0 x-coordinate of first vertex
 * @param {Number} y0 y-coordinate of first vertex
 * @param {Number} z0 z-coordinate of first vertex
 * @param {Number} x1 x-coordinate of second vertex
 * @param {Number} y1 y-coordinate of second vertex
 * @param {Number} z1 z-coordinate of second vertex
 * @chainable
 * @example
 * <div>
 * <code>
 * //draw a line
 * function setup(){
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw(){
 *   background(200);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   // Use fill instead of stroke to change the color of shape.
 *   fill(255, 0, 0);
 *   line(10, 10, 0, 60, 60, 20);
 * }
 * </code>
 * </div>
 */
p5.RendererGL.prototype.line = function(x0,y0,z0,x1,y1,z1) {
  if (typeof x0 !== 'undefined' ||
      typeof y0 !== 'undefined' ||
      typeof z0 !== 'undefined' ||
      typeof x1 !== 'undefined' ||
      typeof y1 !== 'undefined' ||
      typeof z1 !== 'undefined')
  {
    this.beginShape();
    this.vertex(x0, y0, z0);
    this.vertex(x1, y1, z1);
    this.endShape();
  }
  return this;
};

module.exports = p5;

},{"../core/core":54,"./p5.Geometry":101}],107:[function(_dereq_,module,exports){


module.exports = {
  immediateVert:
    "attribute vec3 aPosition;\nattribute vec4 aVertexColor;\n\nuniform mat4 uModelViewMatrix;\nuniform mat4 uProjectionMatrix;\nuniform float uResolution;\nuniform float uPointSize;\n\nvarying vec4 vColor;\nvoid main(void) {\n  vec4 positionVec4 = vec4(aPosition * vec3(1.0, -1.0, 1.0), 1.0);\n  gl_Position = uProjectionMatrix * uModelViewMatrix * positionVec4;\n  vColor = aVertexColor;\n  gl_PointSize = uPointSize;\n}\n",
  vertexColorVert:
    "attribute vec3 aPosition;\nattribute vec4 aVertexColor;\n\nuniform mat4 uModelViewMatrix;\nuniform mat4 uProjectionMatrix;\n\nvarying vec4 vColor;\n\nvoid main(void) {\n  vec4 positionVec4 = vec4(aPosition * vec3(1.0, -1.0, 1.0), 1.0);\n  gl_Position = uProjectionMatrix * uModelViewMatrix * positionVec4;\n  vColor = aVertexColor;\n}\n",
  vertexColorFrag:
    "precision mediump float;\nvarying vec4 vColor;\nvoid main(void) {\n  gl_FragColor = vColor;\n}",
  normalVert:
    "attribute vec3 aPosition;\nattribute vec3 aNormal;\nattribute vec2 aTexCoord;\n\nuniform mat4 uModelViewMatrix;\nuniform mat4 uProjectionMatrix;\nuniform mat3 uNormalMatrix;\n\nvarying vec3 vVertexNormal;\nvarying highp vec2 vVertTexCoord;\n\nvoid main(void) {\n  vec4 positionVec4 = vec4(aPosition * vec3(1.0, -1.0, 1.0), 1.0);\n  gl_Position = uProjectionMatrix * uModelViewMatrix * positionVec4;\n  vVertexNormal = vec3( uNormalMatrix * aNormal );\n  vVertTexCoord = aTexCoord;\n}\n",
  normalFrag:
    "precision mediump float;\nvarying vec3 vVertexNormal;\nvoid main(void) {\n  gl_FragColor = vec4(vVertexNormal, 1.0);\n}",
  basicFrag:
    "precision mediump float;\nvarying vec3 vVertexNormal;\nuniform vec4 uMaterialColor;\nvoid main(void) {\n  gl_FragColor = uMaterialColor;\n}",
  lightVert:
    "attribute vec3 aPosition;\nattribute vec3 aNormal;\nattribute vec2 aTexCoord;\n\nuniform mat4 uModelViewMatrix;\nuniform mat4 uProjectionMatrix;\nuniform mat3 uNormalMatrix;\nuniform int uAmbientLightCount;\nuniform int uDirectionalLightCount;\nuniform int uPointLightCount;\n\nuniform vec3 uAmbientColor[8];\nuniform vec3 uLightingDirection[8];\nuniform vec3 uDirectionalColor[8];\nuniform vec3 uPointLightLocation[8];\nuniform vec3 uPointLightColor[8];\nuniform bool uSpecular;\n\nvarying vec3 vVertexNormal;\nvarying vec2 vVertTexCoord;\nvarying vec3 vLightWeighting;\n\nvec3 ambientLightFactor = vec3(0.0, 0.0, 0.0);\nvec3 directionalLightFactor = vec3(0.0, 0.0, 0.0);\nvec3 pointLightFactor = vec3(0.0, 0.0, 0.0);\nvec3 pointLightFactor2 = vec3(0.0, 0.0, 0.0);\n\nvoid main(void){\n\n  vec4 positionVec4 = vec4(aPosition, 1.0);\n  gl_Position = uProjectionMatrix * uModelViewMatrix * positionVec4;\n\n  vec3 vertexNormal = vec3( uNormalMatrix * aNormal );\n  vVertexNormal = vertexNormal;\n  vVertTexCoord = aTexCoord;\n\n  vec4 mvPosition = uModelViewMatrix * vec4(aPosition, 1.0);\n  vec3 eyeDirection = normalize(-mvPosition.xyz);\n\n  float shininess = 32.0;\n  float specularFactor = 2.0;\n  float diffuseFactor = 0.3;\n\n  for(int i = 0; i < 8; i++){\n    if(uAmbientLightCount == i) break;\n    ambientLightFactor += uAmbientColor[i];\n  }\n\n  for(int j = 0; j < 8; j++){\n    if(uDirectionalLightCount == j) break;\n    vec3 dir = uLightingDirection[j];\n    float directionalLightWeighting = max(dot(vertexNormal, dir), 0.0);\n    directionalLightFactor += uDirectionalColor[j] * directionalLightWeighting;\n  }\n\n  for(int k = 0; k < 8; k++){\n    if(uPointLightCount == k) break;\n    vec3 loc = uPointLightLocation[k];\n    vec3 lightDirection = normalize(loc - mvPosition.xyz);\n\n    float directionalLightWeighting = max(dot(vertexNormal, lightDirection), 0.0);\n    pointLightFactor += uPointLightColor[k] * directionalLightWeighting;\n\n    //factor2 for specular\n    vec3 reflectionDirection = reflect(-lightDirection, vertexNormal);\n    float specularLightWeighting = pow(max(dot(reflectionDirection, eyeDirection), 0.0), shininess);\n\n    pointLightFactor2 += uPointLightColor[k] * (specularFactor * specularLightWeighting\n      +  directionalLightWeighting * diffuseFactor);\n  }\n\n  if(!uSpecular){\n    vLightWeighting =  ambientLightFactor + directionalLightFactor + pointLightFactor;\n  }else{\n    vLightWeighting = ambientLightFactor + directionalLightFactor + pointLightFactor2;\n  }\n\n}\n",
  lightTextureFrag:
    "precision mediump float;\n\nuniform vec4 uMaterialColor;\nuniform sampler2D uSampler;\nuniform bool isTexture;\n\nvarying vec3 vLightWeighting;\nvarying highp vec2 vVertTexCoord;\n\nvoid main(void) {\n  if(!isTexture){\n    gl_FragColor = vec4(vec3(uMaterialColor.rgb * vLightWeighting), uMaterialColor.a);\n  }else{\n    vec4 textureColor = texture2D(uSampler, vVertTexCoord);\n    if(vLightWeighting == vec3(0., 0., 0.)){\n      gl_FragColor = textureColor;\n    }else{\n      gl_FragColor = vec4(vec3(textureColor.rgb * vLightWeighting), textureColor.a);\n    }\n  }\n}"
};
},{}]},{},[45])(45)
});
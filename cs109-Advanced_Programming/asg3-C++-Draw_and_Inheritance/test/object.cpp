/* $Id: object.cpp,v 1.10 2011-02-14 22:03:59-08 dmfrank - $
 * Derek Frank, dmfrank@ucsc.edu
 *
 * NAME
 *    object - implementation file
 *
 * DESCRIPTION
 *    Implementation of the object class and various classes that
 *    inherit from the object class.
 */

#include <typeinfo>

using namespace std;

#include "object.h"
#include "util.h"

#define WHOAMI \
        "[" << typeid(*this).name() << "@" << (void *) this << "]"

#define CTRACE(ARGS) \
        TRACE ('c', WHOAMI << " " << ARGS)

#define DTRACE(ARGS) \
        TRACE ('d', WHOAMI << " coords=" << coords \
               << " angle=" << angle << endl << ARGS);

//
// dtor - object
//    Destructor inherited by all object based classes.
//
object::~object () {
   CTRACE ("delete");
}

//
// ctor - text
//    Constructor for the text class.
//
text::text (const string &font, const points &size, const string &data):
      fontname(font), fontsize(size), textdata(data) {
   CTRACE ("font=" << fontname << " size=" << fontsize
           << " \"" << textdata << "\"")
}

//
// ctor - ellipse
//    Constructor for the ellipse class.
//
ellipse::ellipse (const inches &initheight, const inches &initwidth,
                  const points &initthick):
      shape(initthick), height(initheight), width(initwidth) {
   CTRACE ("height=" << height << " width=" << width
           << " thick=" << thick);
}

//
// ctor - circle
//    Constructor for the circle class.
//
circle::circle (const inches &diameter, const points &thick):
      ellipse (diameter, diameter, thick) {
}

//
// ctor - polygon
//    Constructor for the polygon class.
//
polygon::polygon (const coordlist &coords, const points &initthick):
      shape(initthick), coordinates(coords) {
   CTRACE ( "thick=" << thick << " coords=" << endl
            << coordinates);
}

//
// ctor - rectangle
//    Constructor for the rectangle class.
//
rectangle::rectangle (const inches &height, const inches &width,
                      const points &initthick):
      polygon (make_list (height, width), initthick) {
}

//
// ctor - square
//    Constructor for the square class.
//
square::square (const inches &width, const points &thick):
      rectangle (width, width, thick) {
}

//
// ctor - line
//    Constructor for the line class.
//
line::line (const inches &length, const points &initthick):
      polygon (make_list (length), initthick) {
      
}

//
// draw - text
//    Function to draw text.
//
void text::draw (ostream &out, const xycoords &coords,
                const degrees &angle) {
   DTRACE ("font=" << fontname << " size=" << fontsize
           << " \"" << textdata << "\"");

   string indent = "   ";

   out << "gsave" << endl
       << indent << "/" << fontname << " findfont" << endl
       << indent << double (fontsize) << " scalefont setfont" << endl
       << indent << double (coords.first) << " "
                 << double (coords.second) << " translate" << endl
       << indent << double (angle) << " rotate" << endl
       << indent << "0 0 moveto" << endl
       << indent << "(" << textdata << ")" << endl
       << indent << "show" << endl
       << "grestore" << endl;
}

//
// draw - ellipse
//    Function to draw an ellipse.
//
void ellipse::draw (ostream &out, const xycoords &coords,
                const degrees &angle) {
   DTRACE ("height=" << height << " width=" << width
           << " thick=" << thick);

   string indent = "   ";
   double xscale, yscale, radius;
   if ( double (height) < double (width)) {
      xscale = 1;
      yscale = double (height) / double (width);
      radius = double (width) / 2;
   } else {
      xscale = double (width) / double (width);
      yscale = 1;
      radius = double (height) / 2;
   }
   
   out << "gsave" << endl
       << indent << "newpath" << endl
       << indent << "/save matrix currentmatrix def" << endl
       << indent << double (coords.first) << " "
                 << double (coords.second) << " translate" << endl
       << indent << double (angle) << " rotate" << endl
       << indent << xscale << " "  << yscale  << " scale" << endl
       << indent << "0 0 " << radius << " 0 360 arc" << endl
       << indent << "save setmatrix" << endl
       << indent << double (thick) << " setlinewidth" << endl
       << indent << "stroke" << endl
       << "grestore" << endl;
}

//
// draw - polygon
//    Function to draw a polygon.
//
void polygon::draw (ostream &out, const xycoords &coords,
                const degrees &angle) {
   DTRACE ( "thick=" << thick << " coords=" << endl
            << coordinates);

   string indent = "   ";

   out << "gsave" << endl
       << indent << "newpath" << endl
       << indent << double (coords.first) << " "
                 << double (coords.second) << " translate" << endl
       << indent << double (angle) << " rotate" << endl
       << indent << "0 0 moveto" << endl;

   coordlist::const_iterator itor = coordinates.begin ();
   for (; itor != coordinates.end (); ++itor) {
      out << indent << double (itor->second) << " "
                    << double (itor->first) << " rlineto" << endl;
   }

   out << indent << "closepath" << endl
       << indent << double (thick) << " setlinewidth" << endl
       << indent << "stroke" << endl
       << "grestore" << endl;
      
}

//
// make_list - rectangle
//    Function returns a list of xycoords.  A rectangle has three
//    coordinates to be put into a list.  The first position is assumed
//    to be (0,0) and is specified at the time the draw command is
//    issued.
//
coordlist rectangle::make_list (
            const inches &height, const inches &width) {
   coordlist coordlist;
   inches nheight (-(double(height)/PTS_PER_INCH));
   coordlist.push_back (xycoords (height, inches (0)));
   coordlist.push_back (xycoords (inches (0),width));
   coordlist.push_back (xycoords (nheight, inches (0)));
   return coordlist;
}

//
// make_list - line
//    Function returns a list of xycoords.  A line is drawn horizontally
//    to the right and has one point after the assumed (0,0).
//
coordlist line::make_list (const inches &length) {
   coordlist coordlist;
   coordlist.push_back (xycoords (inches (0), length));
   return coordlist;
}


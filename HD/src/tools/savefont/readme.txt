savefont.scm is a Gimp script.
To use it, place it in <Home>/.gimp-2.6/script, restart Gimp and you'll find it under File/Create/Save font as images

Given a font, a size in pixels and a destination directory, it creates a set of png images for each character (ascii code ranging from 33 to 125).
Built-in features which are not accessible directly :
- created characters are white
- created characters are anti-aliased (using partial transparency)
- pngs are cropped horizontally to the exact size of the character, and vertically to the maximum height occupied by font characters (unless the font is ill-defined)


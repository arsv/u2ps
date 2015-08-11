/* The actual type of fontvariants is char*[][], i.e. arbitrary-length
   array of (pointers to) arbitrary-length arrays of strings.

   Sadly C does not allow to initialize a structure like that statically,
   even though it is totally possible. So instead, we use doubly-included
   file (this one specifically) to build it.
 
   Do NOT attempt to compile this file! Compile u2ps_data.c instead. */

defont(Courier,
        "R:Courier",
        "B:Courier-Bold",
        "I:Courier-Oblique",
        "O:Courier-BoldOblique" )

defont(FreeMono, 
        "R:FreeMono",
        "B:FreeMonoBold",
        "I:FreeMonoOblique",
        "O:FreeMonoBoldOblique")

defont(DejaVu,
        "R:DejaVuSansMono-Regular",
        "B:DejaVuSansMono-Bold",
        "I:DejaVuSansMono-Oblique",
        "O:DejaVuSansMono-BoldOblique")

defont(EnvyCodeR,
        "R:EnvyCodeR",
        "B:EnvyCodeRBold",
        "I:EnvyCodeRItalic")

defont(Liberation,
        "R:LiberationMono",
        "B:LiberationSans-Bold",
        "I:LiberationSans-Italic",
        "O:LiberationSans-BoldItalic")

defont(FiraMono,
        "R:FiraMono-Regular",
        "B:FiraMono-Bold")

defont(RobotoMono,
        "R:RobotoMono-Regular",
        "B:RobotoMono-Bold",
        "I:RobotoMono-Italic",
        "O:RobotoMono-BoldItalic")

defont(Meslo,
        "R:MesloLGM-Regular",
        "B:MesloLGM-Bold",
        "I:MesloLGM-Italic",
        "O:MesloLGM-BoldItalic")

defont(Iosevka,
        "R:Iosevka",
        "B:Iosevka-Bold",
        "I:Iosevka-Italic",
        "O:Iosevka-BoldItalic")

defont(Unifont,
        "R:UnifontMedium")

defont(Sawarabi,
        "C:SawarabiGothic-Medium")

defont(Tlwg,
        "R:TlwgMono",
        "B:TlwgMono-Bold",
        "I:TlwgMono-Oblique",
        "O:TlwgMono-BoldOblique",
        "T:TlwgMono")

/* vim: set ft=c: */

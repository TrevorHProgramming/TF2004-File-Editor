# TF2004-File-Editor

Current version: 0.1

An editor for the various proprietary file formats used in Transformers (2004) (PS2). 

This program will be able to convert model, texture, and other binary files used in Transformers to a more conventional form so they can be edited and converted back to the game's files. So far there are three file types that are understood well enough for this to be a possibility. 

File types:

VBIN: Most likely "Vertex Binary". The game's model files for characters. The majority can be converted to STL, but animation and texture data needs to be handled differently. Possible that .3ds or another format works better, but STL is simpler and far more portable.

VBIN.Mesh: The game's model files for levels. These are structured significantly differently from standard VBIN files and aren't well understood yet.

ITF: Might be "Infogrames Texture Format/File". The game's texture files, fairly similar to standard .BMPs. 

BDB: "Binary Database." Stores data for enemies, pickups, command nodes, etc for each level. Fairly simple but work on these has barely started. Tied closely with the TDB files, "Text Database," which store the same information but more human-friendly.


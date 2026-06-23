
# Edge-cases to handle

* SSE ignores certain presence bits on vertex descriptions and assumes certain attributes are always present; this affects naive rendering of LOD land. LOD land has incorrect vertex descriptions; see [here](https://github.com/Nukem9/skyrimse-test/blob/master/skyrim64_test/src/patches/CKSSE/BSShaderResourceManager_CK.cpp#L60).

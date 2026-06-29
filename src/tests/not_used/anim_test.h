        void animTest() {
            //Quaternion.identity
            int modelId = 12;
            int animId = 68;
            //int modelId = 11;
            //int animId = 11;

            PakFile animPak("original/LISTANIM.PAK");
            Animation anim1;
            Animation anim2;
            {
                auto& data = animPak.readBlock(animId);
                anim1 = loadAnimation(animId, data.data());
            }
            {
                auto& data = animPak.readBlock(16);
                anim2 = loadAnimation(0, data.data());
            }

            PakFile bodyPak("original/LISTBODY.PAK");
            auto h = bodyPak.headers[modelId];
            auto& testBody = bodyPak.readBlock(modelId);
            auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
            saveModelGLTF(model, { &anim1 }, "data/test");
        }
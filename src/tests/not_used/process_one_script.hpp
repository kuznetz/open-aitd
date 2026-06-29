        void processTestScript() {
            PakFile lifePak("original/LISTLIFE.PAK");
            vector<LifeInstructions> allLifes;
            //int i2 = 514;
            for (int i = 0; i < lifePak.headers.size(); i++)
            {
                auto& data = lifePak.readBlock(i);
                auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
                allLifes.push_back(life);

            }

            int n = 0;
            LifeInstructionsP lifep;
            auto& life = allLifes[n];
            auto lifeData = life.data();
            for (int j = 0; j < life.size(); j++) {
                lifep.push_back(lifeData + j);
            }
            auto& nodes = lifeOptimize(lifep);

            ofstream out(string("data/life_")+to_string(n)+".lua", ios::trunc | ios::out);
            out << "function life_" << n << "(obj)\n";
            writeLifeNodes(out, 1, nodes);
            out << "end\n\n";
            out.close();

            /*ofstream out("data/all_life.lua", ios::trunc | ios::out);
            for (int j = 0; j < lifesNodes.size(); j++)
            {
                out << "function life_" << j << "(obj)\n";
                writeLifeNodes(out, 1, lifesNodes[j]);
                out << "end\n\n";
            }
            out.close();*/
        }
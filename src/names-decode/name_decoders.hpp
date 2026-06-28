#pragma once
#include "./name_collection.hpp"

namespace openAITD {

    using namespace ::std;

    class NameDecoders {
    public:
        NameCollection var = NameCollection("V_");
        NameCollection life = NameCollection("L_");
        NameCollection model = NameCollection("M_");
        NameCollection obj = NameCollection("O_");

        NameDecoders() {          
        }

        void load() {
            var.load("newdata/names/vars.json");
            life.load("newdata/names/lifes.json");
            model.load("newdata/names/models.json");
            obj.load("newdata/names/objects.json");
        }
    };

}
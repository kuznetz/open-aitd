#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "./tracks_controller.h"
#include "./player_controller.h"
#include "./hit_controller.h"
#include "./throw_controller.h"
#include "../screens/found_screen.h"

#include "./life_controller/life_core.hpp"
#include "./life_controller/life_commands.hpp"

#include <luacpp/luacpp.h>
#include <iostream>
#include <string>

using namespace std;
using namespace luacpp;

namespace openAITD {

	class LifeController: public LifeCore {
	public:

	  LifeCommands commands;

		LifeController(World* world, TracksController* trackContr, PlayerController* playerContr, HitController* hitContr, ThrowController* throwContr, PhysicsController* physContr, FoundScreen* foundScreen):
			LifeCore(world),
			commands(this, trackContr, playerContr, hitContr, throwContr, physContr, foundScreen)
		{
		}

		~LifeController() {
		}

	};

}

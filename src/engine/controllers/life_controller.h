#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

#include "./music_controller.hpp"
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

		LifeController(World* world, TracksController* trackContr, HitController* hitContr, ThrowController* throwContr, PhysicsController* physContr, FoundScreen* foundScreen, ShootController* shootContr, MusicController* musicContr):
			LifeCore(world),
			commands(this, trackContr, hitContr, throwContr, physContr, foundScreen, shootContr, musicContr)
		{
		}

		~LifeController() {
		}

	};

}

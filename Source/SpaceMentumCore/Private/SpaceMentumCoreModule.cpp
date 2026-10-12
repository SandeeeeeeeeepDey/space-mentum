/**
 * @file SpaceMentumCoreModule.cpp
 * @brief Registers SpaceMentumCore as the PRIMARY game module of the "SpaceMentum" project.
 */

#include "SpaceMentumCoreModule.h"

DEFINE_LOG_CATEGORY(LogSpaceMentum);

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, SpaceMentumCore, "SpaceMentum");

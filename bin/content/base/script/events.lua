-- Currently, we only have dummy functions which call the C functions.
-- Eventually, we'll want to convert everything to Lua.

function arilou_entrance_event()
	custom.arilou_entrance_event()
end

function arilou_exit_event()
	custom.arilou_exit_event()
end

function hyperspace_encounter_event()
	custom.hyperspace_encounter_event()
end

function kohr_ah_victorious_event()
	custom.kohr_ah_victorious_event()
end

function advance_pkunk_mission()
	custom.advance_pkunk_mission()
end

function advance_thradd_mission()
	custom.advance_thradd_mission()
end

function zoqfot_distress_event()
	custom.zoqfot_distress_event()
end

function zoqfot_death_event()
	custom.zoqfot_death_event()
end

function shofixti_return_event()
	custom.shofixti_return_event()
end

function advance_utwig_supox_mission()
	custom.advance_utwig_supox_mission()
end

function kohr_ah_genocide_event()
	custom.kohr_ah_genocide_event()
end

function spathi_shield_event()
	custom.spathi_shield_event()
end

function advance_ilwrath_mission()
	custom.advance_ilwrath_mission()
end

function advance_mycon_mission()
	custom.advance_mycon_mission()
end

function arilou_umgah_check()
	custom.arilou_umgah_check()
end

function yehat_rebel_event()
	custom.yehat_rebel_event()
end

function slylandro_ramp_up()
	custom.slylandro_ramp_up()
end

function slylandro_ramp_down()
	custom.slylandro_ramp_down()
end

function registerEventHandlers()
	event.register("ARILOU_ENTRANCE_EVENT", arilou_entrance_event)
	event.register("ARILOU_EXIT_EVENT", arilou_exit_event)
	event.register("HYPERSPACE_ENCOUNTER_EVENT", hyperspace_encounter_event)
	event.register("KOHR_AH_VICTORIOUS_EVENT", kohr_ah_victorious_event)
	event.register("ADVANCE_PKUNK_MISSION", advance_pkunk_mission)
	event.register("ADVANCE_THRADD_MISSION", advance_thradd_mission)
	event.register("ZOQFOT_DISTRESS_EVENT", zoqfot_distress_event)
	event.register("ZOQFOT_DEATH_EVENT", zoqfot_death_event)
	event.register("SHOFIXTI_RETURN_EVENT", shofixti_return_event)
	event.register("ADVANCE_UTWIG_SUPOX_MISSION", advance_utwig_supox_mission)
	event.register("KOHR_AH_GENOCIDE_EVENT", kohr_ah_genocide_event)
	event.register("SPATHI_SHIELD_EVENT", spathi_shield_event)
	event.register("ADVANCE_ILWRATH_MISSION", advance_ilwrath_mission)
	event.register("ADVANCE_MYCON_MISSION", advance_mycon_mission)
	event.register("ARILOU_UMGAH_CHECK", arilou_umgah_check)
	event.register("YEHAT_REBEL_EVENT", yehat_rebel_event)
	event.register("SLYLANDRO_RAMP_UP", slylandro_ramp_up)
	event.register("SLYLANDRO_RAMP_DOWN", slylandro_ramp_down)
end

log.debug("Registering game event handlers.");
registerEventHandlers()


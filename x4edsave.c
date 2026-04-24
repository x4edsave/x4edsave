// this file modifies the save file from X4:Foundations v9.0 Beta
#
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#define MAX_LINE_LENGTH 65535

int index_until_match(char *line, const char *word) {
	char *found = strstr(line, word);
	if (found) {
		int length = (found - line) + strlen(word);
		return length;
	}
	return 0;
}

int beginson(const char *line, const char *prefix) {
	if (!line || !prefix) return 0;
	size_t len_prefix = strlen(prefix);
	if (strncmp(line, prefix, len_prefix) == 0) {
		return 1;
	}
	return 0;
}

void replace_in_place(char *line, const char *old_w, const char *new_w) {
	char temp[1000];
	char *p = strstr(line, old_w);
	if (p) {
		int prefix_len = p - line;
		memcpy(temp, line, prefix_len);
		strcpy(temp + prefix_len, new_w);
		strcat(temp, p + strlen(old_w));
		strcpy(line, temp);
	}
}

int main(int argc, char *argv[]) {
	SYSTEMTIME system_time;
	DWORD start, end, diff;
	FILE *output_file;
	FILE *input_file;
	char line[MAX_LINE_LENGTH];
	char temp_line[MAX_LINE_LENGTH];
	char* text;
	char* pos_ptr;
	int length;
	int midx;
	char output_filename[256];
	long lines_removed = 0;
	long lines_total = 0;
	
	// REMOVE LOG LINES
	int inside_economylog = 0;
	int end_economylog = 0;
	int inside_log = 0;
	int end_log = 0;
	int inside_tickercache = 0;
	int end_tickercache = 0;
	//
	
	int max_length = 0;
	int index = 0;
	
	WORD this_npc = 0;
	WORD this_people = 0;
	
	WORD skip_one_line = 0;
	WORD skip_lines = 0;
	
	int firstline = 0;
	long in_connection = 0;
	long in_station_connection = 0;
	long in_docked_connection = 0;
	long stations_found = 0;
	long count_player_ships = 0;
	long count_ship_xl = 0;
	long count_ship_l = 0;
	long count_ship_m = 0;
	long count_ship_s = 0;
	long count_xenon_ships = 0;
	long count_xenon_ship_xl = 0;
	long count_xenon_ship_l = 0;
	long count_xenon_ship_m = 0;
	long count_xenon_ship_s = 0;
	long count_drone = 0;
	long count_total_ship = 0;
	
	WORD miner_liquid = 0;
	WORD miner_solid = 0;
	WORD trans_container = 0;
	
	WORD in_universe = 0;
	WORD this_player_ship = 0;
	WORD this_xenon_ship = 0;
	WORD skip_ship = 0;
	WORD in_ship_xl = 0;
	WORD in_ship_l = 0;
	WORD in_ship_m = 0;
	WORD in_ship_s = 0;
	WORD in_ship_connection = 0;
	WORD mod_chassi_xl = 0;
	WORD mod_shields_xl = 0;
	WORD mod_turret_xl = 0;
	WORD mod_weapon_xl = 0;
	WORD mod_chassi_m = 0;
	WORD mod_shields_m = 0;
	WORD mod_shields_m_on = 0;
	WORD mod_turret_m = 0;
	WORD mod_weapon_m = 0;
	WORD mod_chassi_s = 0;
	WORD mod_shields_s = 0;
	WORD mod_shields_s_on = 0;
	WORD mod_turret_s = 0;
	WORD mod_weapon_s = 0;
	WORD manticore = 0;
	WORD skip_orders_manticore = 0;
	WORD transport = 0;
	WORD in_unknown_ship_connection = 0;
	WORD in_unknown_ship_people = 0;
	WORD unknown_ship_people_service = 0;
	WORD unknown_ship_people_marine = 0;
	WORD remove_defence_in_station = 0;
	WORD remove_shields_in_station = 0;
	WORD remove_groups_in_station = 0;
	WORD remove_shieldgen_in_station = 0;
	WORD remove_turret_in_station = 0;
	WORD remove_masstraffic = 0;
	WORD remove_cargo_in_station = 0;
	WORD remove_cargo_in_station_on = 0;
	WORD hack_build_modules = 0;
	WORD xenon_add_resources = 0;
	WORD xenon_add_resources_in = 0;
	WORD xenon_add_cargo_in = 0;
	WORD xenon_shipyard = 0;
	WORD remove_tolerance_xenon = 0;
	
	if (argc < 2) {
		printf("Using: %s <save_00X.xml> \"ALL\" (or selectively) \"nologs xenonup nomods nodef nocargo\" \n", argv[0]);
		return 1;
	}
    
	// Создаем имя для выходного файла
	snprintf(output_filename, sizeof(output_filename), "%s.tmp", argv[1]);
    
	// Открываем исходный файл
	input_file = fopen(argv[1], "rb");
	if (input_file == NULL) {
		printf("Error: don't open file '%s'\n", argv[1]);
		return 1;
	}
    
	// Открываем временный файл для записи
	output_file = fopen(output_filename, "wb");
	if (output_file == NULL) {
		printf("Error: don't create a temp-file\n");
		fclose(input_file);
		return 1;
	}
    
    // Обрабатываем файл
    //printf("Обработка файла...\n");
	
	WORD XENON_POWERUP = 0;
	WORD REMOVE_ECONOMY_LOGS = 0;
	WORD REMOVE_MODIFICATIONS_OTHER_SHIPS = 0;
	WORD REMOVE_DEFENCE_IN_STATIONS = 0;
	WORD REMOVE_CARGO_IN_STATIONS = 0;
	WORD LOG_SHIPS_PLAYER = 0;
	WORD EXPORT_SHIPS_PLAYER = 0;
	printf("The parameters are used:\n");
	printf(" - In-game Modifications are added to the player's ships\n");
	if (argc > 2) {
		if (strstr(argv[2], "xenonup"))				XENON_POWERUP = 1;
		if (strstr(argv[2], "nologs"))				REMOVE_ECONOMY_LOGS = 1;
		if (strstr(argv[2], "nomods"))				REMOVE_MODIFICATIONS_OTHER_SHIPS = 1;
		if (strstr(argv[2], "nodef"))				REMOVE_DEFENCE_IN_STATIONS = 1;
		if (strstr(argv[2], "nocargo"))				REMOVE_CARGO_IN_STATIONS = 1;
		if (strstr(argv[2], "logshipsplayer"))		LOG_SHIPS_PLAYER = 1;
		if (strstr(argv[2], "exportshipsplayer"))	EXPORT_SHIPS_PLAYER = 1;
		
		if (strstr(argv[2], "ALL"))	{
			XENON_POWERUP = 1;
			REMOVE_ECONOMY_LOGS = 1;
			REMOVE_MODIFICATIONS_OTHER_SHIPS = 1;
			REMOVE_DEFENCE_IN_STATIONS = 1;
			REMOVE_CARGO_IN_STATIONS = 1;
			LOG_SHIPS_PLAYER = 1;
		}
		
		if(XENON_POWERUP) printf(" - In-game Modifications are added to the xenon's ships\n");
		if(REMOVE_ECONOMY_LOGS) printf(" - Economy Logs are being deleted\n");
		if(REMOVE_MODIFICATIONS_OTHER_SHIPS) printf(" - In-game Modifications are removed from ships of all factions except...\n");
		if(REMOVE_DEFENCE_IN_STATIONS) printf(" - Protection of stations of all fractions except is removed...\n");
		if(REMOVE_CARGO_IN_STATIONS) printf(" - All Cargo is removed at stations of all fractions except...\n\n");
	}
	
	if(EXPORT_SHIPS_PLAYER) {
		XENON_POWERUP = 0;
		REMOVE_ECONOMY_LOGS = 0;
		REMOVE_MODIFICATIONS_OTHER_SHIPS = 0;
		REMOVE_DEFENCE_IN_STATIONS = 0;
		REMOVE_CARGO_IN_STATIONS = 0;
		LOG_SHIPS_PLAYER = 0;
	}
	
	in_universe = 0;
	in_ship_xl = 0;
	in_ship_l = 0;
	in_ship_m = 0;
	in_ship_s = 0;
	in_ship_connection = 0;
	
	while (fgets(line, sizeof(line), input_file) != NULL) {
        lines_total++;
	
		
		if ( in_universe ) {
			if ( beginson(line, "</universe>")) { in_universe = 0; }
		} else {
			if ( beginson(line, "<universe>")) { in_universe = 1; }	
		}
		
		
		if (in_universe) {
			
			//text = line;
			midx = 0;
			if ( beginson(line, "<component class=\"ship_")) {		// unknown ship
				
				count_total_ship++;
				
				in_unknown_ship_connection = 1;
				skip_ship = 1;
				this_player_ship = 0;
				this_xenon_ship = 0;
				remove_tolerance_xenon = 0;
				this_npc = 0;
				this_people = 0;
				in_ship_connection = 0;
				in_ship_xl = 0;
				in_ship_l = 0;
				in_ship_m = 0;
				in_ship_s = 0;
				mod_shields_m_on = 0;
				skip_one_line = 0;
				skip_lines = 0;
				manticore = 0;
				miner_liquid = 0;
				miner_solid = 0;
				trans_container = 0;
				
				if ( strstr(line, "drone_") == NULL ) {		//this no DRONE
				
					midx = index_until_match(line, "owner=\"player\"");
					if ( midx ) {
						
						skip_ship = 0;
						count_player_ships++;
						in_unknown_ship_connection = 0;
						in_ship_connection = 1;
						this_player_ship = 1;
						
						/*
						if ( strstr(line, "imperator") != NULL ) {
							skip_ship = 1;
						}
						*/
						
					if(LOG_SHIPS_PLAYER) { // ship owner player -> text log
						if ( !skip_ship ) {

							printf("<edit@lines: %ld /> ", lines_total);
							printf("%.*s\n", midx, line);
							
						}
					}
					
					}
				
					if(XENON_POWERUP) {
						if ( strstr(line, "owner=\"xenon\"") != NULL ) {
							
							skip_ship = 0;
							count_xenon_ships++;
							in_unknown_ship_connection = 0;
							in_ship_connection = 1;
							this_xenon_ship = 1;

						}
					}

					//	transports & miners
					if (strstr(line, "_miner_liquid_") != NULL ) miner_liquid = 1;
					if (strstr(line, "_miner_solid_") != NULL ) miner_solid = 1;
					if (strstr(line, "_trans_container_") != NULL ) trans_container = 1;
					
					if ( beginson(line, "<component class=\"ship_xl\"")) {		// ship xl
						
						in_ship_l = 1;
						in_ship_xl = 1;
						count_ship_xl += this_player_ship;
						count_xenon_ship_xl += this_xenon_ship;
					}
				
					if ( beginson(line, "<component class=\"ship_l\"")) {		// ship l
						// xl = l
						in_ship_l = 1;
						in_ship_xl = 1;
						count_ship_l += this_player_ship;
						count_xenon_ship_l += this_xenon_ship;
						
						if (this_player_ship) {
							if ( strstr(line, "_l_trans_container_") != NULL ) {		//transport add drones
								transport = 2;
							}
						}
						
					}
				
					if ( beginson(line, "<component class=\"ship_m\"")) {		// ship m
						
						in_ship_m = 1;
						count_ship_m += this_player_ship;
						count_xenon_ship_m += this_xenon_ship;
						
						if (this_player_ship) {
							manticore = 0;
							if ( strstr(line, "ship_gen_m_tugboat_01_a_macro") != NULL ) {		//manticore -> regiondamage
								manticore = 1;
							}
						}
						
					}
					
					if ( beginson(line, "<component class=\"ship_s\"")) {		// ship s
						
						in_ship_s = 1;
						count_ship_s += this_player_ship;
						count_xenon_ship_s += this_xenon_ship;
					}
				
				}
				
			}	// unknown ship
			
			if ( beginson(line, "<component class=\"station\"")) {		// unknown station
				
				this_player_ship = 0;
				this_xenon_ship = 0;
				in_ship_connection = 0;
				in_station_connection = 1;
				
				remove_groups_in_station = 0;
				remove_shields_in_station = 0;
				remove_shieldgen_in_station = 0;
				remove_turret_in_station = 0;
				
				hack_build_modules = 0;
				
				xenon_add_resources = 0;
				xenon_add_resources_in = 0;
				xenon_add_cargo_in = 0;
				xenon_shipyard = 0;
				remove_defence_in_station = 0;
				remove_cargo_in_station = 0;
				
				if(REMOVE_DEFENCE_IN_STATIONS) {
					remove_defence_in_station = 1;
				}
				if(REMOVE_CARGO_IN_STATIONS) {
					remove_cargo_in_station = 1;
				}

				if ( strstr(line, "owner=\"player\"") != NULL ) {
					
					remove_defence_in_station = 0;
					remove_cargo_in_station	= 0;
				}

				if(XENON_POWERUP) {
					if ( strstr(line, "owner=\"xenon\"") != NULL ) {
						
						remove_defence_in_station = 0;
						remove_cargo_in_station	= 0;
						xenon_add_resources = 1;
						
						if ( strstr(line, "station_xen_shipyard") != NULL ) {
							xenon_shipyard = 1;
							
						}
					}
				}
				
			}
			
			
			if ( in_ship_connection ) {
				
				if ( beginson(line, "<component ")) {
					if ( strstr(line, "/>") == NULL ) {
						in_ship_connection++;
					}
				}
				
				if ( beginson(line, "</component>")) {
					
						if ( in_ship_connection > 0 ) in_ship_connection--;
				
				}
				
				if ( beginson(line, "<hull value=")) {	//repair hull - set @ modifications
					if ( strstr(line, "/>") != NULL ) {
						skip_one_line = 1;
					}
				}

				
				// SKILLS
				
				if (!this_npc) {
					if ( beginson(line, "<component class=\"npc\"")) {
						
						this_npc = 1;
						
					}
				} else {
					if ( beginson(line, "<skills ")) {
						skip_one_line = 1;
						
						//printf(">changes :%ld >>> %s\n", lines_total, line);
						
						fprintf(output_file, "<skills boarding=\"15\" engineering=\"15\" management=\"15\" morale=\"15\" piloting=\"15\"/>\n");
					}
					if ( beginson(line, "</traits>")) {
						this_npc = 0;
					}
				}
				
				if (!this_people) {
					if ( beginson(line, "<people>")) {
						
						this_people = 1;
						
					}
				} else {
					if ( beginson(line, "<skills ")) {
						skip_one_line = 1;
						
						//printf(">changes :%ld >>> %s\n", lines_total, line);
						
						fprintf(output_file, "<skills boarding=\"15\" engineering=\"15\" management=\"15\" morale=\"15\" piloting=\"15\"/>\n");
					}
					if ( beginson(line, "</people>")) {
						this_people = 0;
					}					
				}

				if(XENON_POWERUP) {
					if (this_xenon_ship) {
						
						if ( beginson(line, "<hull value=")) {
							if ( strstr(line, "/>") != NULL ) {
								skip_one_line = 1;
							}
						}
						
						if ( beginson(line, "<skills ")) {
							skip_one_line = 1;
							
							//printf(">changes xenon ship:%ld >>> %s\n", lines_total, line);
							
							fprintf(output_file, "<skills boarding=\"15\" engineering=\"15\" management=\"15\" morale=\"15\" piloting=\"15\"/>\n");
						} 
						
						if (!remove_tolerance_xenon) {
							if ( beginson(line, "<tolerance>")) {
								//printf(">remove tolerance in xenon ship:%ld >>> %s\n", lines_total, line);
								remove_tolerance_xenon = 1;
								skip_lines = 1;
							}
						} else {
							if ( beginson(line, "</tolerance>")) {
								
								remove_tolerance_xenon = 0;
								skip_lines = 0;
								skip_one_line = 1;
							}
						}
						
					}
				}
				
				// ^SKILLS
			

				if ( in_ship_s ) {
					
					if ( mod_chassi_s ){
						if ( beginson(line, "</modification>")) {
							skip_lines = 0;
							mod_chassi_s = 0;
						}
					}
					
					if ( !mod_chassi_s && beginson(line, "<modification>")) {
						skip_lines = 1;
						mod_chassi_s = 1;
						fprintf(output_file, "<hull value=\"9999999\"/>\n");
						fprintf(output_file, "<modification>\n");
						fprintf(output_file, "<engine ware=\"mod_engine_boostthrust_01_mk3\" forwardthrust=\"1.3\" strafethrust=\"1.15\" boostthrust=\"1.3\" boostduration=\"1.3\" boostacc=\"1.3\"/>\n");
						fprintf(output_file, "<paint ware=\"paintmod_0006\" generated=\"1\"/>\n");
						fprintf(output_file, "<ship ware=\"mod_ship_regiondamage_01_mk3\" maxhull=\"1.2\" regiondamage=\"1\"/>\n");
					}
					
					
					if ( !mod_shields_s_on && !mod_shields_s && beginson(line, "<shields>")) {
						skip_lines = 1;
						mod_shields_s = 1;
						mod_shields_s_on = 1;
						fprintf(output_file, "<shields>\n");
						fprintf(output_file, "<group>\n");
						fprintf(output_file, "<modification ware=\"mod_shield_capacity_02_mk3\" capacity=\"1.7\" rechargedelay=\"0.3\" rechargerate=\"1.3\"/>\n");
						//fprintf(output_file, "<modification ware=\"mod_shield_rechargerate_01_mk3\" rechargedelay=\"0.3\" rechargerate=\"1.7\"/>\n");
						fprintf(output_file, "</group>\n");
						//fprintf(output_file, "</shields>\n");
					}
				
					if ( mod_shields_s && beginson(line, "</shields>")) {
						skip_lines = 0;
						mod_shields_s = 0;
					}	
					
					if ( !mod_shields_s_on && beginson(line, "<ammunition>")) {
						mod_shields_s_on = 1;
						//skip_one_line = 1;
						fprintf(output_file, "<shields>\n");
						fprintf(output_file, "<group>\n");
						fprintf(output_file, "<modification ware=\"mod_shield_capacity_02_mk3\" capacity=\"1.7\" rechargedelay=\"0.3\" rechargerate=\"1.7\"/>\n");
						fprintf(output_file, "</group>\n");
						fprintf(output_file, "</shields>\n");
						//fprintf(output_file, "<ammunition>\n");
						
					}
					
					//weapon
					if ( !mod_weapon_s && beginson(line, "<component class=\"weapon\"")) {
						mod_weapon_s = 1;
						
						//printf(">S weapon> line:%ld %s", lines_total, line);
					}
					
					if ( mod_weapon_s && beginson(line, "<modification ware=")) {
						skip_one_line = 1;
						
					}
					
					if ( mod_weapon_s && beginson(line, "</component>")) {
						skip_one_line = 1;
						fprintf(output_file, "<modification ware=\"mod_weapon_damage_02_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.8\" sticktime=\"1.3\"/>\n");
						fprintf(output_file, "</component>\n");
						mod_weapon_s = 0;
						//printf(">M weapon mod> line:%ld %s", lines_total, line);
					}
					
				}	// in_ship_s
				

				if ( in_ship_m ) {
					
					if ( mod_chassi_m ){
						if ( beginson(line, "</modification>")) {
							skip_lines = 0;
							mod_chassi_m = 0;
						}
					}
					
					if ( !mod_chassi_m && beginson(line, "<modification>")) {
						skip_lines = 1;
						mod_chassi_m = 1;
						
						if ( manticore || miner_liquid || miner_solid || trans_container ) {
							fprintf(output_file, "<hull value=\"9999999\"/>\n");
							fprintf(output_file, "<modification>\n");
							fprintf(output_file, "<engine ware=\"mod_engine_travelthrust_01_mk3\" forwardthrust=\"1.15\" strafethrust=\"1.3\" rotationthrust=\"1.3\" travelthrust=\"1.3\" travelstartthrust=\"1.3\" travelattacktime=\"0.7\" travelchargetime=\"0.7\"/>\n");
							fprintf(output_file, "<paint ware=\"paintmod_0006\" generated=\"1\"/>\n");
							fprintf(output_file, "<ship ware=\"mod_ship_regiondamage_01_mk3\" maxhull=\"1.2\" regiondamage=\"1\"/>\n");
						} else {
							fprintf(output_file, "<modification>\n");
							//fprintf(output_file, "<engine ware=\"mod_engine_boostthrust_01_mk3\" forwardthrust=\"1.3\" strafethrust=\"1.3\" boostthrust=\"1.3\" boostduration=\"1.3\" boostacc=\"1.3\"/>\n");
							fprintf(output_file, "<engine ware=\"mod_engine_forwardthrust_01_mk3\" forwardthrust=\"1.3\" rotationthrust=\"1.3\" boostthrust=\"1.5\" travelthrust=\"1.5\" strafeacc=\"1.5\"/>\n");
							fprintf(output_file, "<paint ware=\"paintmod_0006\" generated=\"1\"/>\n");
							//fprintf(output_file, "<ship ware=\"mod_ship_mass_01_mk3\" mass=\"0.769\" drag=\"0.769\"/>\n");
							fprintf(output_file, "<ship ware=\"mod_ship_radarcloak_01_mk3\" radarrange=\"1.2\" radarcloak=\"-0.8\"/>\n");
							
						}
					}
					
					
					if ( !mod_shields_m_on && !mod_shields_m && beginson(line, "<shields>")) {
						skip_lines = 1;
						mod_shields_m = 1;
						mod_shields_m_on = 1;
						//fputs(line, output_file);
						fprintf(output_file, "<shields>\n");
						fprintf(output_file, "<group>\n");
						//fprintf(output_file, "<modification ware=\"mod_shield_capacity_02_mk3\" capacity=\"1.3\" rechargedelay=\"0.3\" rechargerate=\"1.3\"/>\n");
						fprintf(output_file, "<modification ware=\"mod_shield_rechargerate_01_mk3\" rechargedelay=\"0.3\" rechargerate=\"1.7\"/>\n");
						fprintf(output_file, "</group>\n");
						//fprintf(output_file, "</shields>\n");
					}
				
					if ( mod_shields_m && beginson(line, "</shields>")) {
						skip_lines = 0;
						mod_shields_m = 0;
					}	
					
					if ( !mod_shields_m_on && beginson(line, "<ammunition>")) {
						//skip_one_line = 1;
						mod_shields_m_on = 1;
						fprintf(output_file, "<shields>\n");
						fprintf(output_file, "<group>\n");
						//fprintf(output_file, "<modification ware=\"mod_shield_capacity_02_mk3\" capacity=\"1.3\" rechargedelay=\"0.3\" rechargerate=\"1.3\"/>\n");
						fprintf(output_file, "<modification ware=\"mod_shield_rechargerate_01_mk3\" rechargedelay=\"0.3\" rechargerate=\"1.7\"/>\n");
						fprintf(output_file, "</group>\n");
						fprintf(output_file, "</shields>\n");
						//fprintf(output_file, "<ammunition>\n");
						
					}
					
					
					if ( !manticore ) {
						// turret
						if ( !mod_turret_m && beginson(line, "<component class=\"turret\"")) {
							mod_turret_m = 1;
							
							//printf(">M turret> line:%ld %s", lines_total, line);
						}
						
						if ( mod_turret_m && beginson(line, "<modification ware=")) {
							skip_one_line = 1;
							
						}
						
						if ( mod_turret_m && beginson(line, "</component>")) {
							mod_turret_m = 0;
							skip_one_line = 1;
							fprintf(output_file, "<modification ware=\"mod_weapon_damage_01_mk3\" damage=\"1.7\" cooling=\"1.7\" sticktime=\"1.7\" rotationspeed=\"1.7\"/>\n");
							fprintf(output_file, "</component>\n");
							//printf(">M weapon mod> line:%ld %s", lines_total, line);
						}
						
						//weapon
						/*
						if ( !mod_weapon_m && beginson(line, "<component class=\"missilelauncher\"")) {
							//mod_weapon_m = 1;
							
							//weapon_gen_m_torpedo_01_mk2_macro
							printf(">M missilelauncher -> railgun > line:%ld %s", lines_total, line);
							
							
							replace_in_place(line, "missilelauncher", "weapon");
							replace_in_place(line, "weapon_gen_m_torpedo_01_mk2_macro", "weapon_par_m_railgun_01_mk2_macro");
							
						}
						*/
						
						if ( !mod_weapon_m && beginson(line, "<component class=\"weapon\"")) {
							mod_weapon_m = 1;
						
							//printf(">M weapon> line:%ld %s", lines_total, line);
						}
						
						if ( mod_weapon_m && beginson(line, "<modification ware=")) {
							skip_one_line = 1;
							
						}
						
						if ( mod_weapon_m && beginson(line, "</component>")) {
							mod_weapon_m = 0;
							skip_one_line = 1;
							fprintf(output_file, "<modification ware=\"mod_weapon_damage_02_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.8\" sticktime=\"1.3\"/>\n");
							fprintf(output_file, "</component>\n");
							//printf(">M weapon mod> line:%ld %s", lines_total, line);
						}
						
					}
					
				}	// in_ship_m
				
				if ( in_ship_xl ) {
					
					if ( mod_chassi_xl ){	// next line <gravidar
						if ( beginson(line, "</modification>")) {
							skip_lines = 0;
							mod_chassi_xl = 0;
						}
					}
					
					if ( !mod_chassi_xl && beginson(line, "<modification>")) {
						skip_lines = 1;
						mod_chassi_xl = 1;
						
						if ( miner_liquid || miner_solid || trans_container ) {
							
							fprintf(output_file, "<hull value=\"9999999\"/>\n");
							fprintf(output_file, "<modification>\n");
							fprintf(output_file, "<engine ware=\"mod_engine_travelthrust_01_mk3\" forwardthrust=\"1.15\" strafethrust=\"1.3\" rotationthrust=\"1.3\" travelthrust=\"1.3\" travelstartthrust=\"1.3\" travelattacktime=\"0.7\" travelchargetime=\"0.7\"/>\n");
							fprintf(output_file, "<paint ware=\"paintmod_0006\" generated=\"1\"/>\n");
							fprintf(output_file, "<ship ware=\"mod_ship_regiondamage_01_mk3\" maxhull=\"1.2\" regiondamage=\"1\"/>\n");							
							
						} else {
							
							fprintf(output_file, "<modification>\n");
							fprintf(output_file, "<engine ware=\"mod_engine_travelthrust_01_mk3\" forwardthrust=\"1.15\" strafethrust=\"1.3\" rotationthrust=\"1.3\" travelthrust=\"1.3\" travelstartthrust=\"1.3\" travelattacktime=\"0.7\" travelchargetime=\"0.7\"/>\n");
							fprintf(output_file, "<paint ware=\"paintmod_0006\" generated=\"1\"/>\n");
							fprintf(output_file, "<ship ware=\"mod_ship_mass_01_mk3\" mass=\"0.769\" drag=\"0.769\"/>\n");
							
						}
					}
					
					if ( !mod_shields_xl ) {
						
						if (beginson(line, "<shields>")) {
							skip_lines = 1;
							mod_shields_xl = 1;
							fprintf(output_file, "<shields>\n");
							
						}
						
					}
				
					if ( mod_shields_xl ) {
						
						if (beginson(line, "</shields>")) {
							
							fprintf(output_file, "<group>\n");
							fprintf(output_file, "<modification ware=\"mod_shield_rechargerate_01_mk3\" rechargedelay=\"0.3\" rechargerate=\"1.7\"/>\n");
							//fprintf(output_file, "<modification ware=\"mod_shield_capacity_02_mk3\" capacity=\"1.3\" rechargedelay=\"0.7\" rechargerate=\"1.3\"/>\n");
							fprintf(output_file, "</group>\n");
							
							skip_lines = 0;
							//in_ship_xl = 0;
							mod_shields_xl = 0;
						}
						
						if ( mod_shields_xl && beginson(line, "<group group=\"group")) {
							
							pos_ptr = strstr(line, "/>");
							if ( pos_ptr != NULL ) {
								size_t index = (size_t)(pos_ptr - line);
								memcpy(temp_line, line, index);
								temp_line[index] = '>';
								//temp_line[index+1] = '\r';
								temp_line[index+1] = '\n';
								temp_line[index+2] = '\0';
							
								//printf(">>>> %s\n", temp_line);
							
							} else {
								pos_ptr = strstr(line, ">");
								if ( pos_ptr != NULL ) {
									size_t index = (size_t)(pos_ptr - line);
									memcpy(temp_line, line, index);
									temp_line[index] = '>';
									//temp_line[index+1] = '\r';
									temp_line[index+1] = '\n';
									temp_line[index+2] = '\0';
								}
								
								//printf(">>>> %s\n", temp_line);
								
							}
							fputs(temp_line, output_file);
							fprintf(output_file, "<modification ware=\"mod_shield_rechargerate_01_mk3\" rechargedelay=\"0.3\" rechargerate=\"1.7\">\n");
							//<modification ware="mod_shield_capacity_02_mk3" capacity="1.35" rechargedelay="0.6" rechargerate="1.35">
							
							if ( miner_solid && beginson(line, "<group group=\"group_front"))  {
								fprintf(output_file, "<weapon ware=\"mod_weapon_mining_01_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.9\" mining=\"8.0\"/>\n");
								
							} else {
								fprintf(output_file, "<weapon ware=\"mod_weapon_damage_02_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.9\" sticktime=\"1.3\"/>\n");
								//fprintf(output_file, "<weapon ware=\"mod_weapon_damage_02_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.9\" rotationspeed=\"1.3\"/>\n");
								//fprintf(output_file, "<weapon ware=\"mod_weapon_damage_01_mk3\" damage=\"1.348\" cooling=\"1.358\" sticktime=\"1.3\" rotationspeed=\"1.3\"/>\n");
							}
							
							fprintf(output_file, "</modification>\n");
							fprintf(output_file, "</group>\n");
							
						}
						
					}
				}	//	in_ship_xl	
				
				//weapon L
				
				if (in_ship_l) {

					if ( !mod_weapon_xl && beginson(line, "<component class=\"weapon\"")) {
						mod_weapon_xl = 1;
						
						//printf(">XL weapon> line:%ld %s", lines_total, line);
					}
					/*
					if ( mod_weapon_xl && beginson(line, "<component class=\"dockarea\"")) {
						
						mod_weapon_xl = 0;
						in_ship_l = 0;
						in_ship_xl = 0;
						in_ship_connection = 0;
						
					}
					*/
					
					if ( mod_weapon_xl && beginson(line, "<modification ware=")) {
						skip_one_line = 1;
						
					}
					
					if ( mod_weapon_xl && beginson(line, "</component>")) {
						skip_one_line = 1;
						fprintf(output_file, "<modification ware=\"mod_weapon_damage_02_mk3\" damage=\"1.348\" cooling=\"1.358\" reload=\"1.9\" sticktime=\"1.3\"/>\n");
						fprintf(output_file, "</component>\n");
						mod_weapon_xl = 0;
						//printf(">XL weapon mod> line:%ld %s", lines_total, line);
					}
					
					if ( transport ) {
						
						transport--;
						
						if ( beginson(line, "<item macro=\"ship_gen_xs_repairdrone_01_a_macro")) {
							
							fprintf(output_file, "<item macro=\"ship_gen_xs_repairdrone_01_a_macro\" amount=\"4\"/>\n");
							skip_one_line = 1;
							
							//printf(">L repairdrone> line:%ld %s", lines_total, line);
							
						}
						
						if ( beginson(line, "<item macro=\"ship_gen_xs_cargodrone_empty_01_a_macro")) {
							
							fprintf(output_file, "<item macro=\"ship_gen_xs_cargodrone_empty_01_a_macro\" amount=\"6\"/>\n");
							skip_one_line = 1;
							
							//printf(">L cargodrone> line:%ld %s", lines_total, line);
							
						}
					}
					
				}	//in_ship_l
			
			}	//	in_ship_connection


			if ( in_station_connection ) {

				if(XENON_POWERUP) {
					if (xenon_add_resources) {

						if (beginson(line, "<cargo>")) {
							
							xenon_add_cargo_in = 1;
							fprintf(output_file, "<cargo>\n");
							skip_lines = 1;	
						}
						
						if (xenon_add_cargo_in) {
							
							if (!xenon_shipyard) {
								
								if (beginson(line, "<ware ware=\"energycells")) {
									
									fprintf(output_file, "<ware ware=\"energycells\" amount=\"250000\"/>\n");
								}
								if (beginson(line, "<ware ware=\"ore")) {
									
									fprintf(output_file, "<ware ware=\"ore\" amount=\"250000\"/>\n");
								}
								if (beginson(line, "<ware ware=\"silicon")) {
									
									fprintf(output_file, "<ware ware=\"silicon\" amount=\"250000\"/>\n");
								}
								
							}
							
							if (beginson(line, "</cargo>")) {
								
								if (xenon_shipyard) {
								
									fprintf(output_file, "<ware ware=\"energycells\" amount=\"250000\"/>\n");
									fprintf(output_file, "<ware ware=\"ore\" amount=\"250000\"/>\n");
									fprintf(output_file, "<ware ware=\"silicon\" amount=\"250000\"/>\n");
									
								}
								
								xenon_add_cargo_in = 0;
								skip_lines = 0;
							}
						}
					}
				}

				if(REMOVE_DEFENCE_IN_STATIONS) {
					if (remove_defence_in_station) {
						
						/*
						//// hack_build_modules, time + seconds
						if ( beginson(line, "<component class=\"buildmodule\" macro=\"buildmodule_gen_ships")) {
							
							skip_lines = 1;
							hack_build_modules = 1;
							fputs(line, output_file);
							
						}
						
						if ( hack_build_modules && beginson(line, "<offset>")) {
							
							skip_lines = 0;
							hack_build_modules = 0;
							fprintf(output_file, "<events>\n");
							fprintf(output_file, "<event event=\"hackexpired\" time=\"74000.000\"/>\n");
							fprintf(output_file, "</events>\n");
						}
						//// hack_build_modules
						*/
						
						/*
						if ( beginson(line, "<unit macro=\"ship_gen_xs_repairdrone_01_a_macro\"")) {
							
							skip_one_line = 1;
						}
						
						if ( beginson(line, "<unit macro=\"ship_gen_s_fightingdrone_01_a_macro\"")) {
							
							skip_one_line = 1;
						}
						
						if ( beginson(line, "<item macro=\"ship_gen_xs_repairdrone_01_a_macro\"")) {
							
							skip_one_line = 1;
						}
						
						if ( beginson(line, "<item macro=\"ship_gen_s_fightingdrone_01_a_macro\"")) {
							
							skip_one_line = 1;
						}
						*/
						if ( beginson(line, "<upgrades generated=\"1\">")) {
							
							skip_lines = 1;
							remove_groups_in_station = 1;
						}
						
						if ( remove_groups_in_station ) {
							
							if ( beginson(line, "</upgrades>")) {
								
								remove_groups_in_station = 0;
								//fprintf(output_file, "<groups>\n");
								skip_lines = 0;
								skip_one_line = 1;
								
							}
							
						}
						/*
						if ( beginson(line, "<shields>")) {
							
							skip_lines = 1;
							remove_shields_in_station = 1;
						}
						
						if ( remove_shields_in_station ) {
							
							if ( beginson(line, "</shields>")) {
								
								remove_shields_in_station = 0;
								//fprintf(output_file, "<shields>\n");
								skip_lines = 0;
								skip_one_line = 1;
								
							}
							
						}
						*/
						
						/*
						if ( beginson(line, "<connection connection=\"con_shield")) {
						//if ( beginson(line, "<component class=\"shieldgenerator\"")) {
							skip_lines = 1;
							remove_shieldgen_in_station = 1;
						}
						
						if ( remove_shieldgen_in_station ) {
							
							if ( beginson(line, "</connection>")) {
							//if ( beginson(line, "</component>")) {
								remove_shieldgen_in_station = 0;
								skip_one_line = 1;
								skip_lines = 0;
								
							}
							
						}
						*/
						
						/*
						if ( beginson(line, "<connection connection=\"con_turret")) {
						//if ( beginson(line, "<component class=\"turret\"")) {
							skip_lines = 1;
							remove_turret_in_station = 1;
						}
						
						if ( remove_turret_in_station ) {
							
							if ( beginson(line, "</connection>")) {
							//if ( beginson(line, "</component>")) {
								
								remove_turret_in_station = 0;
								skip_one_line = 1;
								skip_lines = 0;
								
							}
							
						}
						*/
					}
				}

				if(REMOVE_CARGO_IN_STATIONS) {
					if (remove_cargo_in_station) {
						
						if (beginson(line, "<cargo>")) {
							
							remove_cargo_in_station_on = 1;
							skip_lines = 1;	
						}
						
						if (remove_cargo_in_station_on) {
							
							if (beginson(line, "</cargo>")) {
							
								remove_cargo_in_station_on = 0;
								fprintf(output_file, "<cargo>\n");
								skip_lines = 0;	
							}
						}
						
					}
				}
				
				if ( beginson(line, "<connection ")) {
					if ( strstr(line, "/>") == NULL ) {
						in_station_connection++;
					}
				}
				
				if ( beginson(line, "</connection>")) {
					in_station_connection--;
					
				}
				
			}	//in_station_connection

			
			if ( in_unknown_ship_connection ) {
				
				
				if ( beginson(line, "<connection ")) {
					if ( strstr(line, "/>") == NULL ) {
						in_unknown_ship_connection++;
					}
				}
				
				if ( beginson(line, "</connection>")) {
					in_unknown_ship_connection--;
				}
				
				if ( !in_unknown_ship_connection ) {
					
					in_unknown_ship_people = 0;
					unknown_ship_people_service = 0;
					unknown_ship_people_marine = 0;
					
				}
				
				
				// remove modifications
				if(REMOVE_MODIFICATIONS_OTHER_SHIPS) {
					if ( strstr(line, "/>") != NULL ) {
						if (beginson(line, "<engine ware=\"mod_")) {
							skip_one_line = 1;
						}
						
						if (beginson(line, "<ship ware=\"mod_")) {
							skip_one_line = 1;
						}
						
						if (beginson(line, "<weapon ware=\"mod_")) {
							skip_one_line = 1;
						}
						
						if (beginson(line, "<modification ware=\"mod_shield")) {
							skip_one_line = 1;
						}
					}
					
					
					// remove people
					
					if (beginson(line, "<people>")) {
						in_unknown_ship_people = 1;
					}
					if (beginson(line, "</people>")) {
						in_unknown_ship_people = 0;
					}
					
					if ( in_unknown_ship_people ) {
						
						if ( beginson(line, "<person macro=")) {
							
							if ( strstr(line, "role=\"service\">") != NULL ) {
								
								unknown_ship_people_service++;
								
								if ( unknown_ship_people_service > 2 ) {
									
									skip_lines = 1;
									
								}
								
							}
							
						}
						
					}
					
					if ( in_unknown_ship_people ) {
						
						if ( beginson(line, "<person macro=")) {
							
							if ( strstr(line, "role=\"marine\">") != NULL ) {
								
								unknown_ship_people_marine++;
								
								if ( unknown_ship_people_marine > 2 ) {
									
									skip_lines = 1;
									
								}
								
							}
							
						}
						
					}
					
					if ( skip_lines && ( unknown_ship_people_service || unknown_ship_people_marine ) ) {
						
						if ( beginson(line, "</person>" )) {
							
							skip_lines = 0;
							skip_one_line = 1;
							
						}
						
					}
				}
				
			} // in_unknown_ship_connection
		
		}	//	in_universe
		
		///////////////
		///////////////
		if(EXPORT_SHIPS_PLAYER) {
			if (in_ship_connection && in_universe) {
				skip_lines = 0;
			} else {
				skip_lines = 1;
			}
		}
		///////////////
		///////////////	


		///////////////	
		///////////////			
		if(REMOVE_ECONOMY_LOGS) {
			if ( !in_universe ) {
				
				if ( !inside_economylog && !end_economylog ) {
					
					if ( beginson(line, "<economylog>")) {
						
						inside_economylog = 1;
						skip_lines = 1;
						fprintf(output_file, "<economylog>\n");
						fprintf(output_file, "<removed/>\n");
						fprintf(output_file, "<entries type=\"cargo\">\n");
						fprintf(output_file, "</entries>\n");
						fprintf(output_file, "<entries type=\"tradeoffer\">\n");
						fprintf(output_file, "</entries>\n");
						fprintf(output_file, "<entries type=\"trade\">\n");
						fprintf(output_file, "</entries>\n");
						fprintf(output_file, "<entries type=\"money\">\n");
						fprintf(output_file, "</entries>\n");
						
						//printf("\n> economylog, line: %ld\n\n", lines_total);

					}
				} else {
					
					if ( !end_economylog && beginson(line, "</economylog>")) {
						
						inside_economylog = 0;
						end_economylog = 1;
						skip_lines = 0;
					
					}
					
				}
				
				if ( !inside_log && !end_log ) {
					
					if ( beginson(line, "<log>")) {
						
						inside_log = 1;
						skip_lines = 1;
						fprintf(output_file, "<log>\n");
						
						//printf("\n> log, line: %ld\n\n", lines_total);
					}
				} else {
					
					if ( !end_log && beginson(line, "</log>")) {
						
						inside_log = 0;
						end_log = 1;
						skip_lines = 0;
					
					}
					
				}
				/*		
				if ( !inside_tickercache && !end_tickercache ) {
				
					if ( beginson(line, "<tickercache>")) {
						
						inside_tickercache = 1;
						skip_lines = 1;
						fprintf(output_file, "<tickercache>\n");
						
						printf("\n> tickercache, line: %ld\n\n", lines_total);
					}
				} else {
					
					if ( !end_tickercache && beginson(line, "</tickercache>")) {
						
						inside_tickercache = 0;
						end_tickercache = 1;
						skip_lines = 0;
					
					}
					
				}
				*/
			}
			
		}
		///////////////	
		///////////////	
		
		if ( !skip_lines && !skip_one_line ) {
			
			fputs(line, output_file);
			
		} else {
			//if ( beginson(line, "<group group=\"group_")) {
			//if (!skip_one_line)	printf(">skip line> line:%ld %s", lines_total, line);
			//}
			lines_removed++;
			skip_one_line = 0;
		}
	
	}
	
    fclose(input_file);
    fclose(output_file);
    
	/*
    // Заменяем исходный файл обработанным
    if (remove(argv[1]) != 0) {
        printf("Ошибка: Не удалось удалить исходный файл\n");
        remove(output_filename);
        return 1;
    }
    
    if (rename(output_filename, argv[1]) != 0) {
        printf("Ошибка: Не удалось переименовать временный файл\n");
        return 1;
    }
    */
	
    printf("\nLines Total: %ld\n\n", lines_total);
	//printf("Stations Found: %ld\n", stations_found);
	printf("Ships owner player: %ld\n", count_player_ships);
	printf("Ships XL: %ld\n", count_ship_xl);
	printf("Ships L: %ld\n", count_ship_l);
	printf("Ships M: %ld\n", count_ship_m);
	printf("Ships S: %ld\n", count_ship_s);
	printf("Drone S: %ld\n\n", count_drone);
	
	if(XENON_POWERUP) {
		printf("Ships owner xenon: %ld\n", count_xenon_ships);
		printf("Xenon Ships XL: %ld\n", count_xenon_ship_xl);
		printf("Xenon Ships L: %ld\n", count_xenon_ship_l);
		printf("Xenon Ships M: %ld\n", count_xenon_ship_m);
		printf("Xenon Ships S: %ld\n\n", count_xenon_ship_s);
	}
	
	printf("Ships Total: %ld\n", count_total_ship);
	printf("Change/Remove Lines: %ld\n", lines_removed);
    
    return 0;
}

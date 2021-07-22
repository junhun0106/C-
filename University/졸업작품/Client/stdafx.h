// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

//#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <WinSock2.h>
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dwrite.h>
#include <wincodec.h>


#include <d3d11_2.h>
#include <dxgi1_3.h>
#include <d3dx11.h>
#include <D3DX10Math.h>
#include <d3dcompiler.h>
#include <DxErr.h>

//#include <d2d1_1helper.h>
//#include <dwrite.h>

//#include <D3D9Types.h>

#include <Mmsystem.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <atomic>
#include "protocol.h"

using namespace std;

//#define _WITH_MSAA4_MULTISAMPLING

#define FRAME_BUFFER_WIDTH				1024
#define FRAME_BUFFER_HEIGHT				768

#define CUBEMAP_RENDER_TARGET_WIDTH		1024
#define CUBEMAP_RENDER_TARGET_HEIGHT	1024

#define SHADOW_RENDER_TARGET_WIDTH		1024
#define SHADOW_RENDER_TARGET_HEIGHT		1024

#define VS_CB_SLOT_CAMERA				0x00
#define VS_CB_SLOT_WORLD_MATRIX			0x01
#define VS_CB_SLOT_TEXTURE_MATRIX		0x02
#define VS_CB_SLOT_TERRAIN				0x03
#define VS_CB_SLOT_SKYBOX				0x04
#define VS_CB_SLOT_PROJECTION			0x05
#define VS_CB_SLOT_SHADOW				0x06

#define PS_CB_SLOT_LIGHT				0x00
#define PS_CB_SLOT_MATERIAL				0x01
#define PS_CB_SLOT_TERRAIN				0x03
#define PS_CB_SLOT_SKYBOX				0x04

#define PS_SLOT_TEXTURE					0x00
#define PS_SLOT_TEXTURE_TERRAIN			0x02
#define PS_SLOT_TEXTURE_SKYBOX			0x0D
#define PS_SLOT_TEXTURE_CUBEMAPPED		0x0E
#define PS_SLOT_TEXTURE_PROJECTION		0x0F
#define PS_SLOT_TEXTURE_PROJECTED_DEPTH	0x10
#define PS_SLOT_TEXTURE_SHADOW			0x11

#define PS_SLOT_TEXTURE_ZOOM            0x20

#define PS_SLOT_SAMPLER					0x00
#define PS_SLOT_SAMPLER_DETAIL			0x01
#define PS_SLOT_SAMPLER_TERRAIN			0x02
#define PS_SLOT_SAMPLER_SKYBOX			0x04
#define PS_SLOT_SAMPLER_CUBEMAPPED		0x05
#define PS_SLOT_SAMPLER_PROJECTION		0x06
#define PS_SLOT_SAMPLER_SHADOW			0x07
#define PS_SLOT_SAMPLER_SHADOW_PCF		0x08

#define PS_SLOT_SAMPLER_ZOOM            0x08

#define GS_CB_SLOT_CAMERA				0x00

// particle
#define VS_CB_SLOT_PARTICLE				0x07 // ok
#define GS_CB_SLOT_PARTICLE				0x07 // ok
#define GS_CB_SLOT_LIGHT				0x08 // ok

#define PS_SLOT_TEXTURE_PARTICLE		0x12 // ok
#define PS_SLOT_TEXTURE_ARRAY_FP		0x13 // ok
#define GS_SLOT_TEXTURE_RANDOM			0x00 // ok

#define PS_SLOT_SAMPLER_PARTICLE		0x08 // ok
#define GS_SLOT_SAMPLER_PARTICLE		0x00 // ok

// blurring
#define CS_SLOT_TEXTURE					0x00 // TEXTURE, b#
#define CS_SLOT_RW_TEXTURE				0x00 // UAV, u#

// game state
#define GAME_STATE_LOADING		0
#define GAME_STATE_LOBBY		1
#define GAME_STATE_PLAY			2

#define RANDOM_COLOR	D3DXCOLOR((rand() * 0xFFFFFF) / RAND_MAX)

#define _WITH_TERRAIN_PARTITION
#define _WITH_FRUSTUM_CULLING_BY_OBJECT
//#define _WITH_FRUSTUM_CULLING_BY_SUBMESH

//#define _WITH_SKYBOX_TEXTURE_ARRAY
#define _WITH_SKYBOX_TEXTURE_CUBE
//#define _WITH_TERRAIN_TEXTURE_ARRAY

extern void TRACE(_TCHAR *pString);
extern void TRACE(char *pString);
extern void TRACE(_TCHAR *pString, UINT uValue);
extern void TRACE(_TCHAR *pString, int nValue);
extern void TRACE(_TCHAR *pString, int nValue0, int nValue1);
extern void TRACE(_TCHAR *pString, float fValue);

namespace ObjectType
{
	const unsigned int player_weapon = 13;
}

namespace Animation
{
	const unsigned int Idle = 0;
	const unsigned int walk = 1;
	const unsigned int run = 2;
	const unsigned int reload = 3;
	const unsigned int shoot = 4;
}
namespace WeaponType
{
	const unsigned int pistol = 0;
	const unsigned int rifle_ak = 1;
	const unsigned int rifle_m = 2;
	const unsigned int sniper_rifle = 3;
}
//---------------------------------------

#define ANIFRAMETIME			0.0333333f

#define SERVERIP "192.168.43.15"

const auto BUFF_SIZE = 4000;
const auto WM_SOCKET = WM_USER + 1;


namespace Environment {
	const std::string roadLane_straight_Centered_mesh("Assets/roadLane_straight_Centered_mesh.data");
	const std::string roadLane_Nolines_straight_mesh("Assets/roadLane_Nolines_straight_mesh.data");
	const std::string roadLane_NoLines_straight_Centered_mesh("Assets/roadLane_NoLines_straight_Centered_mesh.data");
	const std::string road_t_mesh("Assets/road_t_mesh.data");
	const std::string road_straight_mesh("Assets/road_straight_mesh.data");
	const std::string road_straight_clear_mesh("Assets/road_straight_clear_mesh.data");
	const std::string road_square_mesh("Assets/road_square_mesh.data");
	const std::string road_Roundabout("Assets/road_Roundabout.data");
	const std::string road_LaneTransition_Right("Assets/road_LaneTransition_Right.data");
	const std::string road_LaneTransition_Left("Assets/road_LaneTransition_Left.data");
	const std::string road_divider_mesh("Assets/road_divider_mesh.data");
	const std::string road_crossing_mesh("Assets/road_crossing_mesh.data");
	const std::string road_cornerLines_mesh("Assets/road_cornerLines_mesh.data");
	const std::string road_corner_mesh("Assets/road_corner_mesh.data");
	const std::string road_bend_right_mesh("Assets/road_bend_right_mesh.data");
	const std::string road_bend_left_mesh("Assets/road_bend_left_mesh.data");
	const std::string path_straight_mesh("Assets/path_straight_mesh.data");
	const std::string path_driveway("Assets/path_driveway.data");
	const std::string path_cross_mesh("Assets/path_cross_mesh.data");
	const std::string grass_square2_mesh("Assets/grass_square2_mesh.data");
	const std::string grass_square_mesh("Assets/grass_square_mesh.data");
	const std::string flower_mesh("Assets/flower_mesh.data");
	const std::string Env_Water_Tile("Assets/Env_Water_Tile.data");
	const std::string Env_Seawall_Wall("Assets/Env_Seawall_Wall.data");
	const std::string Env_Seawall_Straight("Assets/Env_Seawall_Straight.data");
	const std::string Env_Seawall_Corner_03("Assets/Env_Seawall_Corner_03.data");
	const std::string Env_Seawall_Corner_02("Assets/Env_Seawall_Corner_02.data");
	const std::string Env_Seawall_Corner_01("Assets/Env_Seawall_Corner_01.data");
	const std::string Env_Rocks_03("Assets/Env_Rocks_03.data");
	const std::string Env_Rocks_02("Assets/Env_Rocks_02.data");
	const std::string Env_Rocks_01("Assets/Env_Rocks_01.data");
	const std::string Env_Road_Ramp_Straight("Assets/Env_Road_Ramp_Straight.data");
	const std::string Env_Road_Ramp_Pillar("Assets/Env_Road_Ramp_Pillar.data");
	const std::string Env_Road_Ramp("Assets/Env_Road_Ramp.data");
	const std::string Env_Road_Corner("Assets/Env_Road_Corner.data");
	const std::string Env_Planter("Assets/Env_Planter.data");
	const std::string Env_Jetty("Assets/Env_Jetty.data");
	const std::string Env_Foot_Bridge("Assets/Env_Foot_Bridge.data");
	const std::string Env_Car_Bridge_02("Assets/Env_Car_Bridge_02.data");
	const std::string Env_Car_Bridge("Assets/Env_Car_Bridge.data");
	const std::string Env_Canal_Straight("Assets/Env_Canal_Straight.data");
	const std::string Env_Canal_Pipe_02("Assets/Env_Canal_Pipe_02.data");
	const std::string Env_Canal_Pipe_01("Assets/Env_Canal_Pipe_01.data");
	const std::string Env_Canal_End("Assets/Env_Canal_End.data");
	const std::string Env_Canal_Corner_03("Assets/Env_Canal_Corner_03.data");
	const std::string Env_Canal_Corner_02("Assets/Env_Canal_Corner_02.data");
	const std::string Env_Canal_Corner_01("Assets/Env_Canal_Corner_01.data");
	const std::string Env_Beach_Straight("Assets/Env_Beach_Straight.data");
	const std::string Env_Beach_Short("Assets/Env_Beach_Short.data");
	const std::string Env_Beach_Corner("Assets/Env_Beach_Corner.data");
};

namespace Property {
	const std::string trash_mesh("Assets/trash_mesh.data");
	const std::string tree_large("Assets/tree_large_mesh.data");
	const std::string tree_medium("Assets/tree_medium_mesh.data");
	const std::string tree_small("Assets/tree_small_mesh.data");
	const std::string traffic_light("Assets/traffic_light_mesh.data");
	const std::string Tree01("Assets/Prop_Tree_01.data");
	const std::string Tree02("Assets/Prop_Tree_02.data");
	const std::string Umbrella1("Assets/Prop_Umbrella_01.data");
	const std::string Umbrella2("Assets/Prop_Umbrella_02.data");
	const std::string Umbrella3("Assets/Prop_Umbrella_03.data");
	const std::string Buoy1("Assets/Props_Buoy_01.data");
	const std::string Buoy2("Assets/Props_Buoy_02.data");
	const std::string RoadSign1("Assets/Prop_Roadsign_01.data");
	const std::string RoadSign2("Assets/Prop_Roadsign_02.data");
	const std::string RoadSign3("Assets/Prop_Roadsign_03.data");
	const std::string TirePile("Assets/Prop_TirePile.data");
	const std::string BeachSeat1("Assets/Prop_Beachseat_01.data");
	const std::string BeachSeat2("Assets/Prop_Beachseat_02.data");
	const std::string BeachSeat3("Assets/Prop_Beachseat_03.data");
	const std::string lamp("Assets/lamp_mesh.data");
	const std::string memorial("Assets/memorial_mesh.data");
	const std::string hydrant("Assets/hydrant_mesh.data");
	const std::string pipe_mesh("Assets/pipe_mesh.data");
	const std::string grave_large("Assets/grave_large_mesh.data");
	const std::string grave_medium("Assets/grave_medium_mesh.data");
	const std::string grave_small("Assets/grave_small_mesh.data");
	const std::string hedge_mesh("Assets/hedge_mesh.data");
	const std::string fence_long("Assets/fence_long_mesh.data");
	const std::string fence_short("Assets/fence_short_mesh.data");
	const std::string fence_long_spike("Assets/fence_long_spike.data");
	const std::string fence_short_spike("Assets/fence_short_spike.data");
	const std::string ariel("Assets/ariel_mesh.data");
	const std::string flag("Assets/flag_mesh.data");
	const std::string dumpter_mesh("Assets/dumpster_mesh.data");
	const std::string dish_mesh("Assets/dish_mesh.data");
	const std::string bush_large_mesh("Assets/bush_large_mesh.data");
	const std::string bush_small_mesh("Assets/bush_small_mesh.data");
	const std::string bin_mesh("Assets/bin_mesh.data");
	const std::string billboard_mesh("Assets/billboard_mesh.data");
};


namespace Vehicle {
	const std::string Ambulance("Assets/ambo_mesh.data");
	const std::string Ambulance_seperate("Assets/ambo_seperate_mesh.data");
	const std::string Bus("Assets/bus_mesh.data");
	const std::string Bus_seperate("Assets/bus_seperate_mesh.data");
	const std::string Cop("Assets/cop_mesh.data");
	const std::string Cop_seperate("Assets/cop_seperate_mesh.data");
	const std::string Car("Assets/car_mesh.data");
	const std::string Car_seperate("Assets/car_seperate_mesh.data");
	const std::string Rubbish_truck("Assets/rubbishTruck_mesh.data");
	const std::string Rubbish_seperate_truck("Assets/rubbish_truck_seperate_mesh.data");
	const std::string taxi("Assets/taxi_mesh.data");
	const std::string van("Assets/van_mesh.data");
	const std::string ute("Assets/ute_mesh.data");
	const std::string ute_empty("Assets/ute_empty_mesh.data");
	const std::string ute_seperate_mesh("Assets/ute_seperate_mesh.data");
	const std::string ute_empty_seperate_mesh("Assets/ute_empty_seperate_mesh.data");
	const std::string fire_truck("Assets/fire_truck_mesh.data");
	const std::string fire_truck_seperate("Assets/fire_truck_seperate_mesh.data");

	const std::string Boat1("Assets/Vehicle_Boat_01.data");
	const std::string Boat2("Assets/Vehicle_Boat_02.data");
	const std::string Boat3("Assets/Vehicle_Boat_03.data");

	const std::string PizzaCar_seperate("Assets/pizza_car_seperate.data");
	const std::string HotdogTruck_seperate("Assets/hotdog_truck_seperate.data");

};


namespace Building
{
	const std::string Apartment_large("Assets/apartment_large_mesh.data");
	const std::string Apartment_small("Assets/apartment_small_mesh.data");
	const std::string BarberShop("Assets/Building_BaberShop.data");
	const std::string Cinema("Assets/Building_Cinema.data");
	const std::string CoffeeShop("Assets/Building_CoffeeShop.data");
	const std::string Garage1("Assets/Building_Garage_01.data");
	const std::string Garage2("Assets/Building_Garage_02.data");
	const std::string Garage3("Assets/Building_Garage_03.data");
	const std::string AutoRepair("Assets/Building_AutoRepair.data");
	const std::string Gym("Assets/Building_Gym.data");

	const std::string House1("Assets/Building_House_01.data");
	const std::string House2("Assets/Building_House_02.data");
	const std::string House3("Assets/Building_House_03.data");
	const std::string House4("Assets/Building_House_04.data");
	const std::string House5("Assets/Building_House_05.data");
	const std::string House6("Assets/Building_House_06.data");
	const std::string House7("Assets/Building_House_07.data");
	const std::string House8("Assets/Building_House_08.data");
	const std::string House9("Assets/Building_House_09.data");
	const std::string House10("Assets/Building_House_10.data");
	const std::string House11("Assets/Building_House_11.data");
	const std::string House12("Assets/Building_House_12.data");
	const std::string House13("Assets/Building_House_13.data");
	const std::string House14("Assets/Building_House_14.data");
	const std::string House15("Assets/Building_House_15.data");

	const std::string Mall_A("Assets/Building_Mall_A.data");
	const std::string Mall_B("Assets/Building_Mall_B.data");
	const std::string Shop1("Assets/Building_Shop_01.data");
	const std::string Shop2("Assets/Building_Shop_02.data");
	const std::string Shop3("Assets/Building_Shop_03.data");
	const std::string Shop4("Assets/Building_Shop_04.data");
	const std::string Shop5("Assets/Building_Shop_05.data");
	const std::string StripClub("Assets/Building_StripClub.data");
	const std::string Store_Corner("Assets/store_corner_mesh.data");
	const std::string House("Assets/house_mesh.data");
	const std::string Office_large("Assets/office_large_mesh.data");
	const std::string Office_medium("Assets/office_medium_mesh.data");
	const std::string Office_small("Assets/office_small_mesh.data");
	const std::string Office_Stepped("Assets/office_stepped_mesh.data");

	const std::string Pizza_Shop("Assets/pizza_shop.data");

	const std::string apartment_large_thin_short("Assets/apartment_large_thin_short.data");//LightTown;
	const std::string store_small("Assets/store_small_mesh.data");
};

namespace Texture
{
	const std::string Building01a_Orange("Textures/Building01a.png");
	const std::string Building01b_Red("Textures/Building01b.png");
	const std::string Building01c_Yellow("Textures/Building01c.png");

	const std::string BuildingVideo("Textures/Building02a.png");
	const std::string BuildingPawn("Textures/Building02b.png");
	const std::string BuildingDrug("Textures/Building02c.png");

	const std::string Building03a_Blue("Textures/Building03a.png");
	const std::string Building03b_Gray("Textures/Building03b.png");
	const std::string Building03c_Beige("Textures/Building03c.png");

	const std::string House_01_Red("Textures/House_01.png");
	const std::string House_02_Green("Textures/House_02.png");
	const std::string House_03_Yellow("Textures/House_03.png");

	const std::string Nature_Trees("Textures/Nature_Trees.png");
	const std::string Props("Textures/Props.png");
	const std::string Props_Billboard("Textures/Props_Billboard.png");
	const std::string Props_Dumpster("Textures/Props_Dumpster.png");
	const std::string Road("Textures/Road.png");
	const std::string Road_divider("Textures/Road_divider.png");
	const std::string Shop_Damaged("Textures/Shop_Damaged.png");
	const std::string SimpleTown("Textures/SimpleTown.png");
	const std::string STL_Building_Apartment_02("Textures/STL_Building_Apartment_02.png");
	const std::string STL_Building_Pizza("Textures/STL_Building_Pizza.png");
	const std::string STL_Props_Billboard_02("Textures/STL_Props_Billboard_02.png");
	const std::string STL_Props_Dumpster("Textures/STL_Props_Dumpster.png");

	const std::string STL_Vehicle_HotdogTruck("Textures/STL_Vehicle_HotdogTruck.png");
	const std::string STL_Vehicle_PizzaCar("Textures/STL_Vehicle_PizzaCar.png");

	const std::string Ambulance("Textures/Vehice_Ambulance.png");
	const std::string Vehicle_Bus_Green("Textures/Vehicle_Bus_a.png");
	const std::string Vehicle_Bus_Gray("Textures/Vehicle_Bus_b.png");
	const std::string Vehicle_Bus_SkyBlue("Textures/Vehicle_Bus_c.png");
	const std::string Vehicle_Car01_Blue("Textures/Vehicle_Car01_a.png");
	const std::string Vehicle_Car01_Red("Textures/Vehicle_Car01_b.png");
	const std::string Vehicle_Car01_Green("Textures/Vehicle_Car01_c.png");
	const std::string Vehicle_Fire_Truck("Textures/Vehicle_Fire_Truck.png");
	const std::string Vehicle_Police("Textures/Vehicle_Police.png");
	const std::string Vehicle_Rubbish_Truck("Textures/Vehicle_Rubbish_Truck.png");
	const std::string Vehicle_Taxi("Textures/Vehicle_Taxi.png");
	const std::string Vehicle_Ute_Red("Textures/Vehicle_Ute_a.png");
	const std::string Vehicle_Ute_Yellow("Textures/Vehicle_Ute_b.png");
	const std::string Vehicle_Ute_Blue("Textures/Vehicle_Ute_c.png");
	const std::string Vehicle_Van_SkyBlue("Textures/Vehicle_Van_a.png");
	const std::string Vehicle_Van_Green("Textures/Vehicle_Van_b.png");
	const std::string Vehicle_Van_Red("Textures/Vehicle_Van_c.png");
};


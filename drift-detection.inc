/*
	Author: 
		Karim 
	Description:
		An include which detects drifting with useful callbacks and functions for whoever working in a drift gamemode, racing system or a minigame.
	Latest Update Date:
		16-Feb-2018
	Version:
		1.1.0
*/

// include guard to avoid duplication
#if 	defined 	_drift_included
	#endinput
#endif

#include <a_samp>

#define _drift_included

#define Drift:: Drift_

// constants
#define MIN_DRIFT_ANGLE     (12.0)   // minimum drift angle to start a drift
#define MAX_DRIFT_ANGLE     (80.0)  // maximum drift angle to start a drift
#define MIN_DRIFT_SPEED     (45.0)  // minimum speed to start a drift

#define DRIFT_PROCESS_INTERVAL    (250)     // delay interval between each tick in ms
#define DRIFT_TIMEOUT_INTERVAL    (6)       // how much ticks before a drift ends after the player stops drifting

// drift ending reasons
enum {
    DRIFT_END_REASON_TIMEOUT, 
    DRIFT_END_REASON_DAMAGED, 
    DRIFT_END_REASON_OTHER 
}
// drift states
enum {
    DRIFT_STATE_NONE,
    DRIFT_STATE_DRIFTING
}

// option flags

enum DriftOptions: (<<= 1) {
    DRIFT_CHECK_ENABLED = 1,
    DAMAGE_CHECK_ENABLED,
}
// player drift structure
enum e_DRIFT_PLAYER_STRUCT {
    Float:  vHealth,
    Float:  startPosX,
    Float:  startPosY,
    Float:  startPosZ,

    driftState,
    startTimestamp,
    lastTimestamp,
    timeoutTicks,

    // options
    DriftOptions: playerFlags
}

// callback forwards
#if defined OnPlayerDriftStart
    forward	OnPlayerDriftStart(playerid);
#endif 

#if defined OnPlayerDriftUpdate
    forward	OnPlayerDriftUpdate(playerid, Float: drift_angle, Float: speed);
#endif

#if defined OnPlayerDriftEnd
    forward	OnPlayerDriftEnd(playerid, reason, Float: distance, time);
#endif

static g_DriftPlayers[MAX_PLAYERS][e_DRIFT_PLAYER_STRUCT];
static DriftOptions: g_DriftFlags = DRIFT_CHECK_ENABLED | DAMAGE_CHECK_ENABLED;

static Float: g_MinDriftAngle = MIN_DRIFT_ANGLE;
static Float: g_MinDriftSpeed = MIN_DRIFT_SPEED;
static g_DriftTimeoutTicks = DRIFT_TIMEOUT_INTERVAL;
 
// internal functions
static bool: IsModelACar(modelid) 
{
	switch(modelid)
	{
		case 430, 446, 452, 453, 454, 472, 473, 484, 493, 595: return false; // Boats.
		case 448, 461, 462, 463, 468, 471, 481, 509, 510, 521, 522, 523, 581, 586: return false; // Bikes
		case 417, 425, 447, 460, 469, 476, 487, 488, 497, 511, 512, 513, 519, 520, 548, 553, 563, 577, 592, 593: return false; // Airplanes
	}
	return true;
}

// hooked callbacks
public OnPlayerUpdate(playerid)
{
    if(g_DriftFlags & DRIFT_CHECK_ENABLED && g_DriftPlayers[playerid][playerFlags] & DRIFT_CHECK_ENABLED)
    {
        if(GetTickCount() > g_DriftPlayers[playerid][lastTimestamp])
        {
            new vehicleID = GetPlayerVehicleID(playerid);
            if(vehicleID && GetPlayerState(playerid) == PLAYER_STATE_DRIVER && IsModelACar(GetVehicleModel(vehicleID)))
            {
                new Float: vX, Float: vY, Float: vZ;
                GetVehicleVelocity(vehicleID, vX, vY, vZ);

                new Float: angle;
                GetVehicleZAngle(vehicleID, angle);

                new Float: speed;
                speed = floatsqroot(vX*vX + vY*vY + vZ*vZ) * 180.0;
                
                new Float: direction;
                direction = atan2(vY, vX);
                direction -= 90.0;
        
                if(direction < 0) direction += 360.0;

                new Float: driftAngle;
                driftAngle = angle - floatabs(direction);

                if(driftAngle > 270.0) driftAngle -= 270.0;
                if(driftAngle < -270.0) driftAngle += 270.0;

                driftAngle = floatabs(driftAngle);
            
                switch(g_DriftPlayers[playerid][driftState])
                {
                    case DRIFT_STATE_NONE:
                    {
                        if(g_MinDriftAngle <= driftAngle <= MAX_DRIFT_ANGLE && speed >= g_MinDriftSpeed)
                        {                        
                            g_DriftPlayers[playerid][driftState] = DRIFT_STATE_DRIFTING;
                            g_DriftPlayers[playerid][startTimestamp] = GetTickCount();

                            GetVehicleHealth(vehicleID, g_DriftPlayers[playerid][vHealth]);
                            GetPlayerPos(playerid, g_DriftPlayers[playerid][startPosX], g_DriftPlayers[playerid][startPosY], g_DriftPlayers[playerid][startPosZ]);
                            
                            #if defined OnPlayerDriftStart
                                OnPlayerDriftStart(playerid);
                            #endif
                        }
                    }
                    case DRIFT_STATE_DRIFTING:
                    {
                        if(g_DriftFlags & DAMAGE_CHECK_ENABLED && g_DriftPlayers[playerid][playerFlags] & DAMAGE_CHECK_ENABLED)
                        {
                            new Float: vehicleHealth;
                            GetVehicleHealth(vehicleID, vehicleHealth);

                            if(vehicleHealth < g_DriftPlayers[playerid][vHealth])
                            {
                                GetPlayerPos(playerid, vX, vY, vZ);

                                g_DriftPlayers[playerid][driftState] = DRIFT_STATE_NONE;
                                g_DriftPlayers[playerid][timeoutTicks] = 0;

                                #if defined OnPlayerDriftEnd
                                    new Float: distance;
                                    distance = GetPlayerDistanceFromPoint(playerid, g_DriftPlayers[playerid][startPosX], g_DriftPlayers[playerid][startPosY], g_DriftPlayers[playerid][startPosZ]);

                                    OnPlayerDriftEnd(playerid, DRIFT_END_REASON_DAMAGED, distance, GetTickCount() - g_DriftPlayers[playerid][startTimestamp]);
                                #endif
                            }
                        }
                        if(g_MinDriftAngle <= driftAngle <= MAX_DRIFT_ANGLE && speed >= g_MinDriftSpeed)
                        {
                            g_DriftPlayers[playerid][timeoutTicks] = 0;
                            
                            #if defined OnPlayerDriftUpdate
                                OnPlayerDriftUpdate(playerid, driftAngle, speed);
                            #endif
                        }
                        else
                        {
                            g_DriftPlayers[playerid][timeoutTicks]++;

                            if(g_DriftPlayers[playerid][timeoutTicks] >= g_DriftTimeoutTicks)
                            {
                                GetPlayerPos(playerid, vX, vY, vZ);
                                
                                g_DriftPlayers[playerid][driftState] = DRIFT_STATE_NONE;
                                g_DriftPlayers[playerid][timeoutTicks] = 0;

                                #if defined OnPlayerDriftEnd
                                    new Float: distance;
                                    distance = GetPlayerDistanceFromPoint(playerid, g_DriftPlayers[playerid][startPosX], g_DriftPlayers[playerid][startPosY], g_DriftPlayers[playerid][startPosZ]);

                                    OnPlayerDriftEnd(playerid, DRIFT_END_REASON_TIMEOUT, distance, GetTickCount() - g_DriftPlayers[playerid][startTimestamp]);
                                #endif
                            }
                        }
                    }
                }
            }
            g_DriftPlayers[playerid][lastTimestamp] = GetTickCount() + DRIFT_PROCESS_INTERVAL;
        }
    }
    #if defined Drift_OnPlayerUpdate
        return Drift_OnPlayerUpdate(playerid);
    #else
        return true;
    #endif
}
public OnPlayerStateChange(playerid, newstate, oldstate)
{
    if(g_DriftPlayers[playerid][driftState] == DRIFT_STATE_DRIFTING && oldstate == PLAYER_STATE_DRIVER)
    {
        new Float: pX, Float: pY, Float: pZ;
        GetPlayerPos(playerid, pX, pY, pZ);

        g_DriftPlayers[playerid][driftState] = DRIFT_STATE_NONE;
        g_DriftPlayers[playerid][timeoutTicks] = 0;

        #if defined OnPlayerDriftEnd
            new Float: distance;
            distance = GetPlayerDistanceFromPoint(playerid, g_DriftPlayers[playerid][startPosX], g_DriftPlayers[playerid][startPosY], g_DriftPlayers[playerid][startPosZ]);

            OnPlayerDriftEnd(playerid, DRIFT_END_REASON_OTHER, distance, GetTickCount() - g_DriftPlayers[playerid][startTimestamp]);
        #endif
    }

    #if defined Drift_OnPlayerStateChange 
        return Drift_OnPlayerStateChange(playerid, newstate, oldstate);
    #else
        return true;
    #endif
}

public OnPlayerDisconnect(playerid, reason)
{
    if(g_DriftPlayers[playerid][driftState] == DRIFT_STATE_DRIFTING)
    {
        new Float: pX, Float: pY, Float: pZ;
        GetPlayerPos(playerid, pX, pY, pZ);
                
        g_DriftPlayers[playerid][driftState] = DRIFT_STATE_NONE;
        g_DriftPlayers[playerid][timeoutTicks] = 0;   

        #if defined OnPlayerDriftEnd
            new Float: distance;
            distance = GetPlayerDistanceFromPoint(playerid, g_DriftPlayers[playerid][startPosX], g_DriftPlayers[playerid][startPosY], g_DriftPlayers[playerid][startPosZ]);

            OnPlayerDriftEnd(playerid, DRIFT_END_REASON_OTHER, distance, GetTickCount() - g_DriftPlayers[playerid][startTimestamp]);
        #endif
    }
    #if defined Drift_OnPlayerDisconnect
        return Drift_OnPlayerDisconnect(playerid, reason);
    #else
        return true;
    #endif
}

// option functions
stock Drift::SetMinAngle(Float: angle)
{
    if(angle < MAX_DRIFT_ANGLE)
    {
        g_MinDriftAngle = angle;
    }
}
stock Float: Drift::GetMinAngle()
{
    return g_MinDriftAngle;
}
stock Drift::SetMinSpeed(Float: speed)
{
    g_MinDriftSpeed = speed;
}
stock Float: Drift::GetMinSpeed()
{
    return g_MinDriftSpeed;
}
stock Drift::SetTimeoutTicks(ticks)
{
    g_DriftTimeoutTicks = ticks;
}
stock Drift::GetTimeoutTicks()
{
    return g_DriftTimeoutTicks;
}
stock Drift::EnableDetection(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        g_DriftPlayers[playerid][playerFlags] |= DRIFT_CHECK_ENABLED;
    }
    else g_DriftFlags |= DRIFT_CHECK_ENABLED;
}
stock Drift::DisableDetection(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        g_DriftPlayers[playerid][playerFlags] &= ~DRIFT_CHECK_ENABLED;
    }
    else g_DriftFlags &= ~DRIFT_CHECK_ENABLED;
}
stock bool: Drift::IsDetectionEnabled(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        return (g_DriftPlayers[playerid][playerFlags] & DRIFT_CHECK_ENABLED) ? true : false;
    }
    else (g_DriftFlags & DRIFT_CHECK_ENABLED) ? true : false;
}
stock Drift::EnableDamageCheck(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        g_DriftPlayers[playerid][playerFlags] |= DAMAGE_CHECK_ENABLED;
    }
    else g_DriftFlags |= DAMAGE_CHECK_ENABLED;
}
stock Drift::DisableDamageCheck(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        g_DriftPlayers[playerid][playerFlags] &= ~DAMAGE_CHECK_ENABLED;
    }
    else g_DriftFlags &= ~DAMAGE_CHECK_ENABLED;
}
stock bool: Drift::IsDamageCheckEnabled(playerid = -1)
{
    if(playerid != -1 && 0 <= playerid < MAX_PLAYERS)
    {
        return (g_DriftPlayers[playerid][playerFlags] & DAMAGE_CHECK_ENABLED) ? true : false;
    }
    else return (g_DriftFlags & DAMAGE_CHECK_ENABLED) ? true : false;
}
stock bool: Drift::IsPlayerDrifting(playerid)
{
    return !!g_DriftPlayers[playerid][driftState];
}

#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect Drift_OnPlayerDisconnect
#if defined Drift_OnPlayerDisconnect
    forward Drift_OnPlayerDisconnect(playerid, reason);
#endif

#if defined _ALS_OnPlayerStateChange  
    #undef OnPlayerStateChange
#else
    #define _ALS_OnPlayerStateChange  
#endif

#define OnPlayerStateChange Drift_OnPlayerStateChange 
#if defined Drift_OnPlayerStateChange 
    forward Drift_OnPlayerStateChange(playerid, newstate, oldstate);
#endif

#if defined _ALS_OnPlayerUpdate
    #undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif

#define OnPlayerUpdate Drift_OnPlayerUpdate
#if defined Drift_OnPlayerUpdate
    forward Drift_OnPlayerUpdate(playerid);
#endif
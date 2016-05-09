#pragma once

#include "Vector2.h"
#include <iostream>
#include "Graph.h"
#include "Solver.h"
#include "Grid.h"

#include "IAgent.h"
#include "sfwdraw.h"

/*
    Example dumb agent.

    State Description:
        

        Cannon:
            SCAN:
                Turns Right
                enemy in sight? -> Set Target, FIRE
            FIRE:
                Fire weapon
                -> SCAN

        Tank:
            Explore:
                If touching Target, set random Target
                Path to Target                
*/

class AutoAgent : public IAgent 
{
    // Cache current battle data on each update
    tankNet::TankBattleStateData current;

    // output data that we will return at end of update
    tankNet::TankBattleCommand tbc;

    // Could use this to keep track of the environment, check out the header.
    Grid map;

    //////////////////////
    // States for turret and tank
    enum TURRET { SCAN, AIM, LOCK, FIRE } turretState = SCAN;
    enum TANK   { SEEK, PURSUE, AVOID   } tankState   = SEEK;

    float randTimer = 0;

    // Active target location to pursue
    Vector2 target;

    ///////
    
	
	
	// Scanning
    void scan()
    {
        Vector2 tf = Vector2::fromXZ(current.cannonForward);  // current forward
        Vector2 cp = Vector2::fromXZ(current.position); // current position

        tbc.fireWish = false;
        // Arbitrarily look around for an enemy.
      //  tbc.cannonMove = tankNet::CannonMovementOptions::RIGHT;

        // Search through the possible enemy targets
        for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
            if (current.tacticoolData[aimTarget].inSight && current.canFire) // if in Sight and we can fire
            {
                target = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownPosition);
             
                if(dot(tf, normal(target-cp)) > .87f)
                    turretState = LOCK;
               break;
            }
    }

	//for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
	//	if (current.tacticoolData[aimTarget].inSight && current.canFire) // if in Sight and we can fire
	//	{
	//		target = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownPosition);

	//		if (dot(tf, normal(target - cp)) > .87f)
	//			turretState = FIRE;
	//		break;
	//	}

	void lock()
	{
		Vector2 tf = Vector2::fromXZ(current.cannonForward);  // current forward
		Vector2 cp = Vector2::fromXZ(current.position); // current position
		
		
		for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
			
			if (current.tacticoolData[aimTarget].inSight)
			{
				if (dot(tf, normal(target - cp)) < 0 || dot(tf, normal(target - cp)) > .99f)
				{
					tbc.cannonMove = tankNet::CannonMovementOptions::LEFT;
				}
				if (dot(tf, normal(target - cp)) < .90f  && dot(tf, normal(target - cp)) > .50f)
				{
					tbc.cannonMove = tankNet::CannonMovementOptions::RIGHT;
				}
				if (dot(tf, normal(target - cp)) < 1.2f && dot(tf, normal(target - cp)) > .95f  && distance(target, cp) < 15)
				{
					turretState = FIRE;
				}
			}


			for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
				if (current.tacticoolData[aimTarget].inSight == false)
					turretState = SCAN;

	}


    void fire()
    {
        // No special logic for now, just shoot.
        tbc.fireWish = current.canFire;
        turretState = LOCK;
    }

    std::list<Vector2> path;
    void seek()
    {
        Vector2 cp = Vector2::fromXZ(current.position); // current position
        Vector2 cf = Vector2::fromXZ(current.forward);  // current forward

        
        randTimer -= sfw::getDeltaTime();

       
		// If we're pretty close to the target, get a new target           
		if (distance(target, cp) < 20 || randTimer < 0)
        {
            target = Vector2::random() * Vector2 { 50, 50 };
			if (target == Vector2{ 1 , -9 } || target == Vector2{ -5 , 5 } || target == Vector2{ 5 , 5 })
			{
				target = Vector2::random() * Vector2 { 50, 50 };
			}
            randTimer = rand() % 4 + 2; // every 2-6 seconds randomly pick a new target
        }


       
        // determine the forward to the target (which is the next waypoint in the path)
        Vector2 tf = normal(target - cp);        

        if (dot(cf, tf) > .87f) // if the dot product is close to lining up, move forward
            tbc.tankMove = tankNet::TankMovementOptions::FWRD;
        //else if (dot(cf, tf) < .50)// otherwise turn right until we line up!
       //     tbc.tankMove = tankNet::TankMovementOptions::LEFT;
		else
			tbc.tankMove = tankNet::TankMovementOptions::RIGHT;

		for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
			if (current.tacticoolData[aimTarget].inSight)
				tankState = PURSUE;
		//if (current.forward == 0)
		//{
		//	map.addObstacle(cp);

//		}
    }

	void pursue()
	{
		Vector2 cp = Vector2::fromXZ(current.position); // current position
		Vector2 cf = Vector2::fromXZ(current.forward);  // current forward


		for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
			if (current.tacticoolData[aimTarget].inSight) 
			{
				target = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownPosition);
			}
			else
				tankState = SEEK;

		Vector2 tf = normal(target - cp);

		if (dot(cf, tf) > .87f) // if the dot product is close to lining up, move forward
			tbc.tankMove = tankNet::TankMovementOptions::FWRD;
	
		else
			tbc.tankMove = tankNet::TankMovementOptions::RIGHT;
		
		int randDir;
		int timeThing = 2;
		//randTimer;
		timeThing -= sfw::getDeltaTime();
	
		if (current.forward == 0)
		{
			map.addObstacle(cp);

		}


		//if (distance(target, cp) < 15)
		//{

		//	if (timeThing < 0)
		//	{
		//		randDir = rand() % 4 + 2;
		//	}

		//	if (randDir = 2)
		//	{
		//		tbc.tankMove = tankNet::TankMovementOptions::LEFT;
		//		timeThing = 2;
		//	}
		//	if (randDir = 3)
		//	{
		//		tbc.tankMove = tankNet::TankMovementOptions::RIGHT;
		//		timeThing = 2;
		//	}

		//	if (randDir = 4)
		//	{
		//		tankState = SEEK;
		//		timeThing = 2;
		//	}
		//	if (randDir = 5)
		//	{
		//		tankState = SEEK;
		//		timeThing = 2;
		//	}
		//	if (randDir = 6)
		//	{
		//		tankState = SEEK;
		//		timeThing = 2;
		//	}
		//}



		

		
		if (distance(target, cp) < 10)
			tankState = AVOID;

	}

	void avoid()
	{
		
		Vector2 cp = Vector2::fromXZ(current.position); // current position
		Vector2 cf = Vector2::fromXZ(current.forward);  // current forward
		
	
		
		


		tbc.tankMove = tankNet::TankMovementOptions::BACK;
		if (distance(target, cp) > 25)
			tankState = SEEK;

	}


public:
    tankNet::TankBattleCommand update(tankNet::TankBattleStateData *state)
    {
        current = *state;
        tbc.msg = tankNet::TankBattleMessage::GAME;

        switch (turretState)
        {
        case SCAN: scan(); break;
		case LOCK: lock(); break;
        case FIRE: fire(); break;
        }

        switch (tankState)
        {
		case PURSUE: pursue(); break;
        case SEEK  : seek (); break;
		case AVOID : avoid(); break;
        }

        return tbc;
    }
};





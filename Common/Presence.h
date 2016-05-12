#ifndef _PRESENCE_H_
#define _PRESENCE_H_

#include <d3dx9.h>
#include "OpenSteer/Boid.h"

//#define TIGER

class Presence : public Boid
{
	public:
		Presence(ProximityDatabase& pd, BoxObstacle* obstacles);
		Presence(D3DXVECTOR3 Position, ProximityDatabase& pd, BoxObstacle* obstacles);
		~Presence();

		D3DXMATRIX GetWorld();

		void SetPosition(D3DXVECTOR3 Position);
		void MoveBy(D3DXVECTOR3 &Vector);

	private:
		void RefreshWorld();

		D3DXMATRIX World;
};

#endif
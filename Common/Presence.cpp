#include "Presence.h"

Presence::Presence(ProximityDatabase& pd, BoxObstacle* obstacles)
: Boid(pd, obstacles)
{
	D3DXMatrixIdentity(&this->World);
}

Presence::Presence(D3DXVECTOR3 Position, ProximityDatabase& pd, BoxObstacle* obstacles)
: Boid(pd, obstacles)
{
	this->SetPosition(Position);
}

Presence::~Presence()
{
}

D3DXMATRIX Presence::GetWorld()
{
	#ifdef TIGER
		World._11 = -this->_side.x;
		World._12 = -this->_side.y;
		World._13 = -this->_side.z;

		World._31 = -this->_forward.x;
		World._32 = -this->_forward.y;
		World._33 = -this->_forward.z;

		World._41 = Position.x;
		World._42 = 1.0f;
		World._43 = Position.z;

		return this->World;
	#else
		D3DXMATRIX Scale;
		D3DXMatrixScaling(&Scale, 0.005f, 0.005f, 0.005f);

		World._11 = -this->_side.x;
		World._12 = -this->_side.y;
		World._13 = -this->_side.z;

		World._21 = 0.0f;
		World._22 = 1.0f;
		World._23 = 0.0f;

		World._31 = -this->_forward.x;
		World._32 = -this->_forward.y;
		World._33 = -this->_forward.z;

		World._41 = Position.x;
		World._42 = 1.8f;
		World._43 = Position.z;

		return Scale * this->World;
	#endif
}

void Presence::SetPosition(D3DXVECTOR3 Position)
{
	this->Position = Vec3(Position.x, Position.y, Position.z);
}

void Presence::MoveBy(D3DXVECTOR3 &Vector)
{
	this->Position += Vec3(Vector.x, Vector.y, Vector.z);
}
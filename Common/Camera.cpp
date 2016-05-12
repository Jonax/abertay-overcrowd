#include "Camera.h"

Camera::Camera()
{
//	this->Position	= D3DXVECTOR3(0.0f, 5.0f, 0.0f);
//	this->Focus		= D3DXVECTOR3(1.0f, 4.0f, -1.0f);

	this->Position	= D3DXVECTOR3(17.0f, 14.0f, 14.0f);
	this->Focus		= D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	this->SetView();
	this->SetProjection();
}

Camera::~Camera()
{
}

//	Function to set the projection matrix.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::SetProjection()
{
	D3DXMatrixPerspectiveFovLH(&this->Projection, D3DXToRadian(60.0f), 1.0f, 0.1f, 50.0f );

	this->UpdateMatrix();
}

void Camera::SetView()
{
	D3DXMatrixLookAtLH(	&this->View,
						&this->Position,
						&this->Focus,
						&D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	this->UpdateMatrix();
}

void Camera::UpdateMatrix()
{
	this->VP = this->View * this->Projection;
}

D3DXMATRIX Camera::GetMatrix()
{
	return this->VP;
}

void Camera::Left(float TimeDelta)
{
	D3DXVECTOR3 Dir = Focus - Position;
	D3DXVec3Normalize(&Dir, &Dir);

	D3DXVec3Cross(&Dir, &Dir, &D3DXVECTOR3(0.0f, 1.0f, 0.0));

	Position	+= Dir * TimeDelta;
	Focus		+= Dir * TimeDelta;

	this->SetView();
}

void Camera::Right(float TimeDelta)
{
	D3DXVECTOR3 Dir = Focus - Position;
	D3DXVec3Normalize(&Dir, &Dir);

	D3DXVec3Cross(&Dir, &Dir, &D3DXVECTOR3(0.0f, -1.0f, 0.0));

	Position	+= Dir * TimeDelta;
	Focus		+= Dir * TimeDelta;

	this->SetView();
}

void Camera::Up(float TimeDelta)
{
	D3DXVECTOR3 Dir = Focus - Position;

	D3DXVec3Normalize(&Dir, &Dir);

	D3DXVec3Cross(&Dir, &Dir, &D3DXVECTOR3(-1.0f, 0.0f, 0.0));

	Position	+= Dir * TimeDelta;
	Focus		+= Dir * TimeDelta;

	this->SetView();
}

void Camera::Down(float TimeDelta)
{
	D3DXVECTOR3 Dir = Focus - Position;
	D3DXVec3Normalize(&Dir, &Dir);

	D3DXVec3Cross(&Dir, &Dir, &D3DXVECTOR3(1.0f, 0.0f, 0.0));

	Position	+= Dir * TimeDelta;
	Focus		+= Dir * TimeDelta;

	this->SetView();
}
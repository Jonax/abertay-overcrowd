#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <d3dx9.h>

class Camera
{
	public:
		Camera();
		~Camera();

		void SetProjection();
		void SetView();

		void Left(float TimeDelta);
		void Right(float TimeDelta);
		void Up(float TimeDelta);
		void Down(float TimeDelta);

		void TurnLeft();
		void TurnRight();

		D3DXMATRIX GetMatrix();

	private:
		void UpdateMatrix();

		D3DXVECTOR3 Position, Focus;
		D3DXVECTOR2 Orient;
		D3DXMATRIX VP, View, Projection;
};

#endif
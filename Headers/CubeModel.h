#pragma once

#include "ObjectModel.h"



class CubeModel : public ObjectModel
{
public:
	CubeModel();
	CubeModel(MyFiles*);
	
	~CubeModel();
		
	bool Update() ; // we supply an update
	bool Draw(); // and a draw 
	
	
	

};

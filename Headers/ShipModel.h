
#pragma once

#include "ObjectModel.h"
//#include "ModelManager.h"



class ShipModel : public ObjectModel
{
public:
	ShipModel();
	ShipModel(char* FileName, MyFiles*);

	~ShipModel();
		
	bool Update();  // we supply an update
	bool Draw();  // and a draw 
	

	
	

};


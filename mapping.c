#include "mapping.h"
#include "window.h"

int mappingMapSet = 0;

int mapping_feedback( pWindow window, pWindowElement elem, int action )
{
	spMapSetMapSet(mappingMapSet);
	if (action == WN_ACT_UPDATE)
		sprintf(elem->text,"%s: %s",spMapCaptionByID(elem->reference),spMapButtonByID(elem->reference));
	spMapSetMapSet(0);
	return 0;
}

void mapping_draw( void )
{
	
}

int mapping_calc( Uint32 steps )
{
	if (spMapContinueChange())
		return 1;
	return 0;
}

void mapping_window(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int mapSet)
{
	mappingMapSet = mapSet;
	pWindow window = create_window(mapping_feedback,font,mapSet?"Ingame button mapping":"Menu button mapping");
	window->only_ok = 1;
	int i;
	for (i = 0; i < 8; i++)
		add_window_element(window,-1,i);
	while (1)
	{
		if (modal_window(window,resize) == 1)
		{
			spMapSetMapSet(mappingMapSet);
			char buffer[256];
			sprintf(buffer,"Press new button for %s!\n(Was %s)",spMapCaptionByID(window->selection),spMapButtonByID(window->selection));
			set_message(font,buffer);
			draw_message();
			spFlip();
			spMapStartChangeByID(window->selection);
			spLoop(mapping_draw,mapping_calc,10,resize,NULL);
			pWindowElement elem;
			for (elem = window->firstElement; elem; elem = elem->next)
				mapping_feedback(window,elem,WN_ACT_UPDATE);
			spMapSetMapSet(0);
		}
		else
			break;
	}
	delete_window(window);
	spMapSetMapSet(mappingMapSet);
	spMapSave("hase",mapSet?"controls.cfg":"menu.cfg");
	spMapSetMapSet(0);
}

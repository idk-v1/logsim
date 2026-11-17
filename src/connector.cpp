#include "connector.h"

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"
#include "logicsim.h"

Connector::Connector(LogicBlock* a, const std::string& aIndex, 
	LogicBlock* b, const std::string& bIndex, bool aIn, bool bIn) :
	a(a), b(b), aIn(aIn), bIn(bIn) 
{
	this->aIndex = a->getConnectionIndex(aIndex, aIn);
	this->bIndex = b->getConnectionIndex(bIndex, bIn);

	if (a->getConnectionIndex(aIndex, aIn) == -1)
		printf("%s.%c[%s] doesn't exist\n", a->label.data(), aIn ? 'i' : 'o', aIndex.data());
	if (b->getConnectionIndex(bIndex, bIn) == -1)
		printf("%s.%c[%s] doesn't exist\n", b->label.data(), bIn ? 'i' : 'o', bIndex.data());
}

void Connector::tick()
{
	std::vector<WriteData>* aport = aIn ? &a->input : &a->output;
	std::vector<WriteData>* bport = bIn ? &b->input : &b->output;

	if ((*aport)[aIndex].data || !(*bport)[bIndex].written)
		(*bport)[bIndex].data = (*aport)[aIndex].data;
	(*bport)[bIndex].written = true;
}

void Connector::draw(SDL_Surface* surface)
{
	if (a->isHidden() || b->isHidden())
		return;

	Uint32 on = rgb(255, 255, 0);
	Uint32 off = rgb(63, 63, 0);
	if (aIn && bIn)
	{
		on = rgb(0, 255, 255);
		off = rgb(0, 63, 63);
	}
	else if (!aIn && !bIn)
	{
		on = rgb(255, 127, 0);
		off = rgb(63, 31, 0);
	}

	std::vector<WriteData>* aport = aIn ? &a->input : &a->output;

	XYCoord p0 = a->getConnectionPos(aIndex, aIn);
	XYCoord p1 = b->getConnectionPos(bIndex, bIn);
	drawLineFromPoints(surface, p0.x, p0.y, p1.x, p1.y, 
		(*aport)[aIndex].data ? on : off);
}
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
		printf("%s.%c[%s] doesn't exist\n", a->getLabel().data(), aIn ? 'i' : 'o', aIndex.data());
	if (b->getConnectionIndex(bIndex, bIn) == -1)
		printf("%s.%c[%s] doesn't exist\n", b->getLabel().data(), bIn ? 'i' : 'o', bIndex.data());
}

void Connector::tick()
{
	WriteData** aport = aIn ? &a->input : &a->output;
	WriteData** bport = bIn ? &b->input : &b->output;

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

	WriteData** aport = aIn ? &a->input : &a->output;

	XYCoord p0 = a->getConnectionPos(aIndex, aIn);
	XYCoord p1 = b->getConnectionPos(bIndex, bIn);
	drawLineFromPoints(surface, p0.x, p0.y, p1.x, p1.y, 
		(*aport)[aIndex].data ? on : off);
}
#include "connector.h"

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"
#include "logicsim.h"

Connector::Connector(LogicBlock* a, const std::string& aIndex, 
	LogicBlock* b, const std::string& bIndex, bool aIn, bool bIn) :
	a(a), b(b)
{
	WriteData* aport = aIn ? &a->input[0] : &a->output[0];
	WriteData* bport = bIn ? &b->input[0] : &b->output[0];

	this->aIndex = a->getConnectionIndex(aIndex, aIn);
	this->bIndex = b->getConnectionIndex(bIndex, bIn);
	if (!aIn)
		this->aIndex |= 0b10000000000000000000000000000000;
	if (!bIn)
		this->bIndex |= 0b10000000000000000000000000000000;

	if (a->getConnectionIndex(aIndex, aIn) == -1)
		printf("%s.%c[%s] doesn't exist\n", a->getLabel().data(), aIn ? 'i' : 'o', aIndex.data());
	if (b->getConnectionIndex(bIndex, bIn) == -1)
		printf("%s.%c[%s] doesn't exist\n", b->getLabel().data(), bIn ? 'i' : 'o', bIndex.data());
}

void Connector::tick()
{
	bool aIn = !(this->aIndex & 0b10000000000000000000000000000000);
	bool bIn = !(this->bIndex & 0b10000000000000000000000000000000);
	WriteData* aport = aIn ? &a->input[0] : &a->output[0];
	WriteData* bport = bIn ? &b->input[0] : &b->output[0];

	int aIndex = this->aIndex;
	if (aIndex < 0)
		aIndex &= ~0b10000000000000000000000000000000;
	int bIndex = this->bIndex;
	if (bIndex < 0)
		bIndex &= ~0b10000000000000000000000000000000;

	if (aport[aIndex].data || !bport[bIndex].written)
		bport[bIndex].data = aport[aIndex].data;
	bport[bIndex].written = true;
}

void Connector::draw(SDL_Surface* surface)
{
	if (a->isHidden() || b->isHidden())
		return;

	Uint32 on = rgb(255, 255, 0);
	Uint32 off = rgb(63, 63, 0);

	bool aIn = !(this->aIndex & 0b10000000000000000000000000000000);
	bool bIn = !(this->bIndex & 0b10000000000000000000000000000000);
	WriteData* aport = aIn ? &a->input[0] : &a->output[0];

	int aIndex = this->aIndex;
	if (aIndex & 0b10000000000000000000000000000000)
		aIndex &= ~0b10000000000000000000000000000000;
	int bIndex = this->bIndex;
	if (bIndex < 0)
		bIndex &= ~0b10000000000000000000000000000000;

	XYCoord p0 = a->getConnectionPos(aIndex, aIn);
	XYCoord p1 = b->getConnectionPos(bIndex, bIn);
	drawLineFromPoints(surface, p0.x, p0.y, p1.x, p1.y, 
		aport[aIndex].data ? on : off);
}
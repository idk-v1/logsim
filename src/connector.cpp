#include "connector.h"

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

Connector::Connector(LogicBlock& a, const std::string& aIndex, 
	LogicBlock& b, const std::string& bIndex, bool aIn, bool bIn) :
	a(a), b(b), aIndex(aIndex), bIndex(bIndex), aIn(aIn), bIn(bIn) 
{
	std::map<std::string, WriteData, strCmpWithArray>* aport = aIn ? &a.input : &a.output;
	std::map<std::string, WriteData, strCmpWithArray>* bport = bIn ? &b.input : &b.output;

	if (isArray(aIndex) && getArrayNumber(aIndex) == -1)
	{
		std::string aIndex2 = aIndex.substr(0, aIndex.size() - 2);
		if (aport->find(arrayToStr(aIndex2, 0)) == aport->end())
			printf("%s.%c[%s] doesn't exist\n", a.label.data(), aIn ? 'i' : 'o', aIndex.data());
	}
	else
	{
		if (aport->find(aIndex) == aport->end())
			printf("%s.%c[%s] doesn't exist\n", a.label.data(), aIn ? 'i' : 'o', aIndex.data());
	}
	if (isArray(bIndex) && getArrayNumber(bIndex) == -1)
	{
		std::string bIndex2 = bIndex.substr(0, bIndex.size() - 2);
		if (bport->find(arrayToStr(bIndex2, 0)) == bport->end())
			printf("%s.%c[%s] doesn't exist\n", b.label.data(), bIn ? 'i' : 'o', bIndex.data());
	}
	else
	{
		if (bport->find(bIndex) == bport->end())
			printf("%s.%c[%s] doesn't exist\n", b.label.data(), bIn ? 'i' : 'o', bIndex.data());
	}
}

void Connector::tick()
{
	std::map<std::string, WriteData, strCmpWithArray>* aport = aIn ? &a.input : &a.output;
	std::map<std::string, WriteData, strCmpWithArray>* bport = bIn ? &b.input : &b.output;

	if (isArray(aIndex) && isArray(bIndex) && getArrayNumber(aIndex) == -1 && 
		getArrayNumber(bIndex) == -1)
	{
		std::string aOut2(aIndex, 0, aIndex.size() - 2);
		std::string bIn2(bIndex, 0, bIndex.size() - 2);
		// copy all
		for (int i = 0;; ++i)
		{
			if (aport->find(arrayToStr(aOut2, i)) != aport->end() &&
				bport->find(arrayToStr(bIn2, i)) != bport->end())
			{
				if ((*aport)[arrayToStr(aOut2, i)].data || !(*bport)[arrayToStr(bIn2, i)].written)
					(*bport)[arrayToStr(bIn2, i)].data = (*aport)[arrayToStr(aOut2, i)].data;
				(*bport)[arrayToStr(bIn2, i)].written = true;
			}
			else
				break;
		}
	}
	else
	{
		if ((*aport)[aIndex].data || !(*bport)[bIndex].written)
			(*bport)[bIndex].data = (*aport)[aIndex].data;
		(*bport)[bIndex].written = true;
	}
}

void Connector::draw(SDL_Surface* surface)
{
	if (a.isHidden() || b.isHidden())
		return;

	Uint32 on = rgb(255, 255, 0);
	Uint32 off = rgb(63, 63, 0);

	std::map<std::string, WriteData, strCmpWithArray>* aport = aIn ? &a.input : &a.output;
	std::map<std::string, WriteData, strCmpWithArray>* bport = bIn ? &b.input : &b.output;

	if (isArray(aIndex) && isArray(bIndex) && 
		getArrayNumber(aIndex) == -1 && getArrayNumber(bIndex) == -1)
	{
		std::string aOut2(aIndex, 0, aIndex.size() - 2);
		std::string bIn2(bIndex, 0, bIndex.size() - 2);
		// copy all
		for (int i = 0;; ++i)
		{
			std::string aIndexed = arrayToStr(aOut2, i);
			std::string bIndexed = arrayToStr(bIn2, i);
			if (aport->find(aIndexed) != aport->end() &&
				bport->find(bIndexed) != bport->end())
			{
				XYCoord p0 = a.getConnectionPos(a.getConnectionIndex(aIndexed, aIn), aIn);
				XYCoord p1 = b.getConnectionPos(b.getConnectionIndex(bIndexed, bIn), bIn);
				drawLineFromPoints(surface, p0.x, p0.y, p1.x, p1.y, 
					(*aport)[aIndexed].data ? on : off);
			}
			else
				break;
		}
	}
	else
	{
		XYCoord p0 = a.getConnectionPos(a.getConnectionIndex(aIndex, aIn), aIn);
		XYCoord p1 = b.getConnectionPos(b.getConnectionIndex(bIndex, bIn), bIn);
		drawLineFromPoints(surface, p0.x, p0.y, p1.x, p1.y, 
			(*aport)[aIndex].data ? on : off);
	}
}
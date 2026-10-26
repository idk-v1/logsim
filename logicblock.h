#pragma once
#include <vector>
#include <string>
#include <list>
#include <map>

static std::string arrayToStr(const std::string& str, size_t i)
{
	return str + "[" + std::to_string(i) + "]";
}

// a[0] -> 0 | a[] -> -1
static int getArrayNumber(const std::string& str)
{
	size_t pos = str.find('[', 0);
	int num = 0;
	bool found = false;
	for (const char* c = str.data() + pos + 1; *c != ']'; ++c)
	{
		num = num * 10 + (*c - '0');
		found = true;
	}
	if (found)
		return num;
	return -1;
}

static bool isArray(const std::string& str)
{
	return str.find('[', 0) != std::string::npos;
}

struct strCmpWithArray
{
	bool operator()(const std::string& a, const std::string& b) const
	{
		if (isArray(a) && isArray(b))
		{
			for (const char* ca = a.data(), *cb = b.data(); *ca && *cb; ++ca, ++cb)
			{
				if (*ca == *cb)
				{
					if (*ca == '[')
					{
						int ai = getArrayNumber(a);
						int bi = getArrayNumber(b);
						return (ai < bi);
					}
				}
				else
					return (*ca < *cb);
			}
		}
		return a < b;
	}
};

struct LogicBlock
{
	std::map<std::string, bool, strCmpWithArray> input;
	std::map<std::string, bool, strCmpWithArray> output;
	std::list<LogicBlock> blocks;
	std::string label;

	LogicBlock() = default;
	LogicBlock(const std::string& label)
	{
		if (label == "and" || label == "or" || label == "xor")
		{
			input["a"] = 0;
			input["b"] = 0;
			output["out"] = 0;
		}
		else if (label == "not" || label == "delay")
		{
			input["in"] = 0;
			output["out"] = 0;
		}
		else if (label == "on")
			output["out"] = 1;
		else if (label == "off")
			output["out"] = 0;
		else {}
		this->label = label;
	}
	LogicBlock(const std::string& in, const std::string& out, const std::string& label)
	{
		const char* start = in.data();
		for (const char* c = start;; ++c)
		{
			if (*c == '|' || *c == 0)
			{
				if (c - start > 0)
				{
					bool inArray = false;
					int arraySize = 0;
					const char* nameEnd = c;
					for (const char* ch = start; ch != c; ++ch)
					{
						if (*ch == '[')
						{
							inArray = true;
							nameEnd = ch;
						}
						else if (*ch == ']')
							inArray = false;
						else if (inArray)
							arraySize = arraySize * 10 + (*ch - '0');
					}
					if (arraySize)
					{
						for (int i = 0; i < arraySize; ++i)
							input[arrayToStr(std::string(start, nameEnd - start), i)] = 0;
					}
					else
						input[std::string(start, nameEnd - start)] = 0;
				}
				start = c + 1;

				if (*c == 0)
					break;
			}
		}

		start = out.data();
		for (const char* c = start;; ++c)
		{
			if (*c == '|' || *c == 0)
			{
				if (c - start > 0)
				{
					bool inArray = false;
					int arraySize = 0;
					const char* nameEnd = c;
					for (const char* ch = start; ch != c; ++ch)
					{
						if (*ch == '[')
						{
							inArray = true;
							nameEnd = ch;
						}
						else if (*ch == ']')
							inArray = false;
						else if (inArray)
							arraySize = arraySize * 10 + (*ch - '0');
					}
					if (arraySize)
					{
						for (int i = 0; i < arraySize; ++i)
							output[arrayToStr(std::string(start, nameEnd - start), i)] = 0;
					}
					else
						output[std::string(start, nameEnd - start)] = 0;
				}
				start = c + 1;

				if (*c == 0)
					break;
			}
		}

		this->label = label;
	}

	void tick()
	{
		if (label == "and")
			output["out"] = input["a"] && input["b"];
		else if (label == "or")
			output["out"] = input["a"] || input["b"];
		else if (label == "xor")
			output["out"] = input["a"] != input["b"];
		else if (label == "not")
			output["out"] = !input["in"];
		else if (label == "on")
			output["out"] = 1;
		else if (label == "off")
			output["out"] = 0;
		else if (label == "delay")
			output["out"] = input["in"];
		else
			for (auto& b : blocks)
				b.tick();
	}

	LogicBlock& add(const LogicBlock& block)
	{
		blocks.push_back(block);
		return blocks.back();
	}

	void print()
	{
		printf("%s\nIn:\n", label.data());
		for (auto& in : input)
			printf("[%s]: %d\n", in.first.data(), (int)in.second);
		printf("Out\n");
		for (auto& out : output)
			printf("[%s]: %d\n", out.first.data(), (int)out.second);
		printf("\n");
	}
};

#define LogicBlockAnd   LogicBlock("and")
#define LogicBlockOr    LogicBlock("or")
#define LogicBlockXor   LogicBlock("xor")
#define LogicBlockNot   LogicBlock("not")
#define LogicBlockDelay LogicBlock("delay")
#define LogicBlockOn    LogicBlock("on")
#define LogicBlockOff   LogicBlock("off")


struct Connector
{
	Connector(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bIn, int mode) :
		a(a), b(b), aOut(aOut), bIn(bIn), mode(mode) {}

	void tick()
	{
		std::map<std::string, bool, strCmpWithArray>* aport = NULL;
		std::map<std::string, bool, strCmpWithArray>* bport = NULL;
		if (mode == 0)
		{
			aport = &a.output;
			bport = &b.input;
		}
		else if (mode == 1)
		{
			aport = &a.input;
			bport = &b.input;
		}
		else if (mode == -1)
		{
			aport = &a.output;
			bport = &b.output;
		}

		if (isArray(aOut) && isArray(bIn) && getArrayNumber(aOut) == -1 && getArrayNumber(bIn) == -1)
		{
			std::string aOut2(aOut, 0, aOut.size() - 2);
			std::string bIn2(bIn, 0, bIn.size() - 2);
			// copy all
			for (int i = 0;; ++i)
			{
				if (aport->find(arrayToStr(aOut2, i)) != aport->end() &&
					bport->find(arrayToStr(bIn2, i)) != bport->end())
					(*bport)[arrayToStr(bIn2, i)] = (*aport)[arrayToStr(aOut2, i)];
				else
					break;
			}
		}
		else
			(*bport)[bIn] = (*aport)[aOut];
	}

	LogicBlock& a, &b;
	std::string aOut, bIn;
	int mode;
};

struct Board
{
	LogicBlock board;
	std::vector<Connector> connections;
	std::vector<Connector> connectionsIn;
	std::vector<Connector> connectionsOut;

	// a.out -> b.in
	void connect(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bIn)
	{
		connections.push_back(Connector(a, aOut, b, bIn, 0));
	}

	// a.in -> a.child.in
	void connectToIn(LogicBlock& a, const std::string& aIn, LogicBlock& b, const std::string& bIn)
	{
		connectionsIn.push_back(Connector(a, aIn, b, bIn, 1));
	}

	// a.child.out -> a.out
	void connectToOut(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bOut)
	{
		connectionsOut.push_back(Connector(a, aOut, b, bOut, -1));
	}

	LogicBlock& add(const LogicBlock& block)
	{
		return board.add(block);
	}

	void tick()
	{
		for (auto& c : connections)
			c.tick();

		for (auto& c : connectionsIn)
			c.tick();

		board.tick();

		for (auto& c : connectionsOut)
			c.tick();
	}
};
#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <algorithm>

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

struct LogicBlock
{
	std::unordered_map<std::string, bool> input;
	std::unordered_map<std::string, bool> output;
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
		std::cout << label << '\n';

		std::vector<std::string> inputs;
		inputs.reserve(input.size());
		for (auto& in : input)
			inputs.push_back(in.first + "|" + std::to_string(in.second));
		std::sort(inputs.begin(), inputs.end(), 
			[](std::string a, std::string b)
			{
				if (isArray(a))
				{
					int ai = getArrayNumber(a);
					a = std::string(a, 0, a.find('[') + 1);
					a += '0' + (ai / 10000 % 10);
					a += '0' + (ai / 1000 % 10);
					a += '0' + (ai / 100 % 10);
					a += '0' + (ai / 10 % 10);
					a += '0' + (ai % 10);
				}
				if (isArray(b))
				{
					int bi = getArrayNumber(b);
					b = std::string(b, 0, b.find('[') + 1);
					b += '0' + (bi / 10000 % 10);
					b += '0' + (bi / 1000 % 10);
					b += '0' + (bi / 100 % 10);
					b += '0' + (bi / 10 % 10);
					b += '0' + (bi % 10);
				}
				return a < b;
			});
		std::cout << "In:\n";
		for (auto& in : inputs)
		{
			std::cout << std::string(in, 0, in.size() - 2) << ": " << in.back() << '\n';
		}

		std::vector<std::string> outputs;
		outputs.reserve(output.size());
		for (auto& out : output)
			outputs.push_back(out.first + "|" + std::to_string(out.second));
		std::sort(outputs.begin(), outputs.end(),
			[](std::string a, std::string b)
			{
				if (isArray(a))
				{
					int ai = getArrayNumber(a);
					a = std::string(a, 0, a.find('[') + 1);
					a += '0' + (ai / 10000 % 10);
					a += '0' + (ai / 1000 % 10);
					a += '0' + (ai / 100 % 10);
					a += '0' + (ai / 10 % 10);
					a += '0' + (ai % 10);
				}
				if (isArray(b))
				{
					int bi = getArrayNumber(b);
					b = std::string(b, 0, b.find('[') + 1);
					b += '0' + (bi / 10000 % 10);
					b += '0' + (bi / 1000 % 10);
					b += '0' + (bi / 100 % 10);
					b += '0' + (bi / 10 % 10);
					b += '0' + (bi % 10);
				}
				return a < b;
			});
		std::cout << "Out:\n";
		for (auto& out : outputs)
		{
			std::cout << std::string(out, 0, out.size() - 2) << ": " << out.back() << '\n';
		}
		std::cout << "\n";
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
		std::unordered_map<std::string, bool>* aport = NULL;
		std::unordered_map<std::string, bool>* bport = NULL;
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
	std::list<Connector> connections;
	std::list<Connector> connectionsIn;
	std::list<Connector> connectionsOut;

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

#include "TestMmu.h"

TestMmu::TestMmu()
{
	m_Memory = new std::vector<uint8_t>(0xFFFF + 1);
}

TestMmu::~TestMmu() {}

uint8_t& TestMmu::Read(uint16_t address, const bool hasPPUAccess)
{
	return m_Memory->at(address);
}

void TestMmu::Write(uint16_t address, uint8_t value, bool rawWrite)
{
	m_Memory->at(address) = value;
}

void TestMmu::Reset()
{
	delete m_Memory;
	m_Memory = new std::vector<uint8_t>(0xFFFF + 1);
}

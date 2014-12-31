#pragma once


class IPersistentStorage
{
public:
    virtual void PersistentStorage_Write(size_t storageOffset, const uint8_t *data, size_t length) = 0;
    virtual void PersistentStorage_Read(size_t storageOffset, uint8_t *dataBuffer, size_t &out_length) = 0;
};

#ifndef TESTS
class ArduinoPersistentStorage : public IPersistentStorage
{
public:
    virtual void PersistentStorage_Write(size_t storageOffset, const uint8_t *data, size_t length) {}
    virtual void PersistentStorage_Read(size_t storageOffset, uint8_t *dataBuffer, size_t &out_length) {}

};
#else
class PersistentStorageMock : public IPersistentStorage
{
    MOCK_METHOD3(PersistentStorage_Write, void(size_t , const uint8_t *, size_t ));
    MOCK_METHOD3(PersistentStorage_Read, void(size_t , uint8_t *, size_t &));
};


#endif //TESTS

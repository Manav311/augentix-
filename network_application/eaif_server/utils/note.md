note.

/* tensor */

class TensorShape
{
public:
    /// Empty (invalid) constructor.
    TensorShape();
    TensorShape(unsigned int numDimensions);
    TensorShape(unsigned int numDimensions, const unsigned int* dimensionSizes);
    TensorShape(std::initializer_list<unsigned int> dimensionSizeList);
    TensorShape(const TensorShape& other);
    TensorShape& operator=(const TensorShape& other);
    unsigned int operator[](unsigned int i) const;
    unsigned int& operator[](unsigned int i);
    bool operator==(const TensorShape& other) const;
    bool operator!=(const TensorShape& other) const;
    unsigned int GetNumDimensions() const { return m_NumDimensions; }
    unsigned int GetNumElements() const;
private:
    std::array<unsigned int, MaxNumOfTensorDimensions> m_Dimensions;
    unsigned int m_NumDimensions;
    void CheckDimensionIndex(unsigned int i) const;
};

class TensorInfo
{
public:
    TensorInfo();
    TensorInfo(const TensorShape& shape,
               DataType dataType,
               float quantizationScale = 0.0f,
               int32_t quantizationOffset = 0);
    TensorInfo(unsigned int numDimensions,
               const unsigned int* dimensionSizes,
               DataType dataType,
               float quantizationScale = 0.0f,
               int32_t quantizationOffset = 0);
    TensorInfo(const TensorShape& shape,
               DataType dataType,
               const std::vector<float>& quantizationScales,
               unsigned int quantizationDim);
    TensorInfo(unsigned int numDimensions,
               const unsigned int* dimensionSizes,
               DataType dataType,
               const std::vector<float>& quantizationScales,
               unsigned int quantizationDim);
    TensorInfo(const TensorInfo& other);

    TensorInfo& operator=(const TensorInfo& other);
    bool operator==(const TensorInfo& other) const;
    bool operator!=(const TensorInfo& other) const;
    const TensorShape& GetShape() const              { return m_Shape; }
    TensorShape& GetShape()                          { return m_Shape; }
    void SetShape(const TensorShape& newShape)       { m_Shape = newShape; }
    unsigned int GetNumDimensions() const            { return m_Shape.GetNumDimensions(); }
    unsigned int GetNumElements() const              { return m_Shape.GetNumElements(); }
    DataType GetDataType() const                     { return m_DataType; }
    void SetDataType(DataType type)                  { m_DataType = type; }
    bool HasMultipleQuantizationScales() const       { return m_Quantization.m_Scales.size() > 1; }
    bool HasPerAxisQuantization() const;
    std::vector<float> GetQuantizationScales() const;
    void SetQuantizationScales(const std::vector<float>& scales);
    float GetQuantizationScale() const;
    void SetQuantizationScale(float scale);
    int32_t GetQuantizationOffset() const;
    void SetQuantizationOffset(int32_t offset);
    Optional<unsigned int> GetQuantizationDim() const;
    void SetQuantizationDim(const Optional<unsigned int>& quantizationDim);
    bool IsQuantized() const;

    /// Check that the types are the same and, if quantize, that the quantization parameters are the same.
    bool IsTypeSpaceMatch(const TensorInfo& other) const;

    unsigned int GetNumBytes() const;
};

class BaseTensor
{
public:
    /// Empty (invalid) constructor.
    BaseTensor();

    /// Constructor from a raw memory pointer.
    /// @param memoryArea - Region of CPU-addressable memory where tensor data will be stored. Must be valid while
    /// workloads are on the fly. Tensor instances do not claim ownership of referenced memory regions, that is,
    /// no attempt will be made by ArmNN to free these memory regions automatically.
    BaseTensor(const TensorInfo& info, MemoryType memoryArea);

    /// Tensors are copyable.
    BaseTensor(const BaseTensor& other);

    /// Tensors are copyable.
    BaseTensor& operator=(const BaseTensor&);

    const TensorInfo& GetInfo() const { return m_Info; }
    TensorInfo& GetInfo() { return m_Info; }
    const TensorShape& GetShape() const { return m_Info.GetShape(); }
    TensorShape& GetShape() { return m_Info.GetShape(); }

    DataType GetDataType() const                    { return m_Info.GetDataType(); }
    unsigned int GetNumDimensions() const { return m_Info.GetNumDimensions(); }
    unsigned int GetNumBytes() const { return m_Info.GetNumBytes(); }
    unsigned int GetNumElements() const { return m_Info.GetNumElements(); }

    MemoryType GetMemoryArea() const { return m_MemoryArea; }

protected:
    /// Protected destructor to stop users from making these
    /// (could still new one on the heap and then leak it...)
    ~BaseTensor() {}

    MemoryType m_MemoryArea;

private:
    TensorInfo m_Info;
};

/* armnnDeserializer */
namespace armnnDeserializer
{
struct BindingPointInfo
{
    armnn::LayerBindingId   m_BindingId;
    armnn::TensorInfo       m_TensorInfo;
};

/* ITfLiteParser */
class ITfLiteParser
    virtual armnn::INetworkPtr CreateNetworkFromBinaryFile(const char* graphFile) = 0;
    virtual armnn::INetworkPtr CreateNetworkFromBinary(const std::vector<uint8_t> & binaryContent) = 0;
    virtual BindingPointInfo GetNetworkInputBindingInfo(size_t subgraphId,
                                                        const std::string& name) const = 0;
    virtual BindingPointInfo GetNetworkOutputBindingInfo(size_t subgraphId,
                                                         const std::string& name) const = 0;
    virtual size_t GetSubgraphCount() const = 0;
    virtual std::vector<std::string> GetSubgraphInputTensorNames(size_t subgraphId) const = 0;
    virtual std::vector<std::string> GetSubgraphOutputTensorNames(size_t subgraphId) const = 0;

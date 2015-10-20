#ifndef PLATFORM_DMA_DEVICE
#define PLATFORM_DMA_DEVICE

#include <stm32f4xx_dma.h>

// TODO: comments
template< std::uintptr_t DMA_stream, uint32_t channel >
class DMA_dev
{
public:
    DMA_dev();
    ~DMA_dev();

    // Type for status flags
    using s_t = uint32_t;

    // Parent type

    struct flags
    {
    private:
        using dma = DMA_dev< DMA_stream, channel >;

    public:
        static constexpr s_t HT     = dma::pick_HT();
        static constexpr s_t TC     = dma::pick_TC();
        static constexpr s_t ERR    = dma::pick_ERR();
    };

    // Role of transfer endpoints
    enum class role
    {
        memory      = 0,
        periphery   = 1
    };

    int set_origin(role r, volatile const uint8_t* address, size_t size);
    int set_destination(role r, volatile uint8_t* address, size_t size);
    auto get_status() const;

    int submit();
    int complete();

    int enable_IRQ(const std::function< void() > &handler, s_t flags = -1);
    int complete_IRQ(s_t flags = -1);

private:
    static constexpr auto pick_stream();
    static constexpr auto pick_stream_no();
    static constexpr auto pick_RCC();
    //static constexpr auto pick_RCC_fn(); // not needed
    static constexpr auto pick_HT();
    static constexpr auto pick_TC();
    static constexpr auto pick_ERR();
    // Interrupts
    static constexpr auto pick_IT();
    static constexpr auto pick_HT_IF();
    static constexpr auto pick_TC_IF();
    static constexpr auto pick_ERR_IF();


    std::pair< role, volatile const uint8_t* > m_origin;
    std::pair< role, volatile uint8_t* > m_destination;
    size_t m_origin_size;
    size_t m_destination_size;
};


template< std::uintptr_t DMA_stream, uint32_t channel  >
DMA_dev< DMA_stream, channel >::DMA_dev()
    :m_origin{ role::memory, nullptr }
    ,m_destination{ role::memory, nullptr }
    ,m_origin_size{ 0 }
    ,m_destination_size{ 0 }
{
    constexpr auto RCC_DMA = pick_RCC();
    RCC_AHB1PeriphClockCmd(RCC_DMA, ENABLE);
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
DMA_dev< DMA_stream, channel >::~DMA_dev()
{
    // TODO: disable interrupts, disable DMA stream?
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::set_origin(role r, volatile const uint8_t* address, size_t size)
{
    if (!address || !size)
        return -1;

    m_origin = { r, address };
    m_origin_size = size;
    return 0;
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::set_destination(role r, volatile uint8_t* address, size_t size)
{
    if (!address || !size)
        return -1;

    m_destination = { r, address };
    m_destination_size = size;
    return 0;
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
auto DMA_dev< DMA_stream, channel >::get_status() const
{
    constexpr auto stream = pick_stream();
    s_t flags = 0;

    if (DMA_GetFlagStatus(stream, flags::HT) == SET) {
        flags |= flags::HT;
    }

    if (DMA_GetFlagStatus(stream, flags::TC) == SET) {
        flags |= flags::TC;
    }

    if (DMA_GetFlagStatus(stream, flags::ERR) == SET) {
        flags |= flags::ERR;
    }

    return flags;
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::submit()
{
    constexpr auto stream = pick_stream();
    DMA_Cmd(stream, DISABLE);

    DMA_InitTypeDef init;
    DMA_StructInit(&init);

    init.DMA_Channel = channel;
    init.DMA_MemoryInc = DMA_MemoryInc_Enable;

    // Encode direction
    if (m_destination.first == role::memory) {
        if (m_destination_size < m_origin_size) {
            // Provided buffer is not large enough
            return -1;
        }

        init.DMA_PeripheralBaseAddr = reinterpret_cast< uint32_t >(m_origin.second);
        init.DMA_Memory0BaseAddr    = reinterpret_cast< uint32_t >(m_destination.second);

        // Assume that transaction forced by destination
        init.DMA_BufferSize = m_destination_size;

        if (m_origin.first == role::memory) {
            init.DMA_DIR = DMA_DIR_MemoryToMemory;
            init.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
        } else {
            init.DMA_DIR = DMA_DIR_PeripheralToMemory;
            init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        }
    } else {
        if (m_origin_size < m_destination_size) {
            // Provided buffer is not large enough
            return -1;
        }

        init.DMA_BufferSize             = m_origin_size;
        init.DMA_Memory0BaseAddr        = reinterpret_cast< uint32_t >(m_origin.second);
        init.DMA_PeripheralBaseAddr     = reinterpret_cast< uint32_t >(m_destination.second);
        init.DMA_DIR                    = DMA_DIR_MemoryToPeripheral;
        init.DMA_PeripheralInc          = DMA_PeripheralInc_Disable;
    }

    // Clear all flags before we go
    DMA_ClearFlag(stream, pick_TC() | pick_HT() | pick_ERR());

    // Clear all pending interrupts
    DMA_ClearITPendingBit(stream, pick_TC_IF() | pick_HT_IF() | pick_ERR_IF());

    DMA_Init(stream, &init);
    DMA_Cmd(stream, ENABLE);

    return 0;
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::complete()
{
    constexpr auto DMA_IT = pick_IT();
    constexpr auto stream = pick_stream();

    DMA_Cmd(stream, DISABLE);
    IRQ_manager::mask(DMA_IT);
    IRQ_manager::unsubscribe(DMA_IT);

    return 0;
}


template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::enable_IRQ(
        const std::function< void() > &handler, s_t flags)
{
    if (!flags)
        return -1;

    constexpr auto DMA_IT = pick_IT();
    constexpr auto stream = pick_stream();

    uint32_t DMA_IT_flags = 0;

    if ((flags & pick_HT()) == pick_HT()) {
        DMA_IT_flags |= DMA_IT_HT;
    }

    if ((flags & pick_TC()) == pick_TC()) {
        DMA_IT_flags |= DMA_IT_TC;
    }

    if ((flags & pick_ERR()) == pick_ERR()) {
        DMA_IT_flags |= (DMA_IT_TE | DMA_IT_FE);
    }

    // Disable all interrupt sources before we go
    constexpr auto to_clear = DMA_IT_TE | DMA_IT_FE | DMA_IT_TC | DMA_IT_HT;

    DMA_ITConfig(stream, to_clear, DISABLE);
    DMA_ITConfig(stream, DMA_IT_flags, ENABLE);

    IRQ_manager::mask(DMA_IT);
    IRQ_manager::clear(DMA_IT);
    IRQ_manager::subscribe(DMA_IT, handler);
    IRQ_manager::unmask(DMA_IT);

    return 0;
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
int DMA_dev< DMA_stream, channel >::complete_IRQ(s_t flags)
{
    constexpr auto DMA_IT = pick_IT();
    constexpr auto stream = pick_stream();
    uint32_t DMA_IT_flags = 0;

    if ((flags & pick_HT()) == pick_HT()) {
        DMA_IT_flags |= pick_HT_IF();
    }

    if ((flags & pick_TC()) == pick_TC()) {
        DMA_IT_flags |= pick_TC_IF();
    }

    if ((flags & pick_ERR()) == pick_ERR()) {
        DMA_IT_flags |= pick_ERR_IF();
    }

    IRQ_manager::mask(DMA_IT);

    if (flags)
        DMA_ClearFlag(stream, flags);

    if (DMA_IT_flags)
        DMA_ClearITPendingBit(stream, DMA_IT_flags);

    IRQ_manager::clear(DMA_IT);
    IRQ_manager::unmask(DMA_IT);

    return 0;
}


// Private members -------------------------------------------------------------

template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_stream()
{
    return reinterpret_cast< DMA_Stream_TypeDef* >(DMA_stream);
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_stream_no()
{
    constexpr auto stream = pick_stream();

    if (stream == DMA1_Stream0 || stream == DMA2_Stream0) {
        return 0;
    } else if (stream == DMA1_Stream1 || stream == DMA2_Stream1) {
        return 1;
    } else if (stream == DMA1_Stream2 || stream == DMA2_Stream2) {
        return 2;
    } else if (stream == DMA1_Stream3 || stream == DMA2_Stream3) {
        return 3;
    } else if (stream == DMA1_Stream4 || stream == DMA2_Stream4) {
        return 4;
    } else if (stream == DMA1_Stream5 || stream == DMA2_Stream5) {
        return 5;
    } else if (stream == DMA1_Stream6 || stream == DMA2_Stream6) {
        return 6;
    } else if (stream == DMA1_Stream7 || stream == DMA2_Stream7) {
        return 7;
    }
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_RCC()
{
    constexpr auto stream = pick_stream();

    /* Reset interrupt pending bits for the selected stream */
    if (stream == DMA1_Stream0
            || stream == DMA1_Stream1
            || stream == DMA1_Stream2
            || stream == DMA1_Stream3
            || stream == DMA1_Stream4
            || stream == DMA1_Stream5
            || stream == DMA1_Stream6
            || stream == DMA1_Stream7) {
        return RCC_AHB1Periph_DMA1;
    } else {
        return RCC_AHB1Periph_DMA2;
    }

}


template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_HT_IF()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_IT_HTIF0;
    case 1:
        return DMA_IT_HTIF1;
    case 2:
        return DMA_IT_HTIF2;
    case 3:
        return DMA_IT_HTIF3;
    case 4:
        return DMA_IT_HTIF4;
    case 5:
        return DMA_IT_HTIF5;
    case 6:
        return DMA_IT_HTIF6;
    case 7:
        return DMA_IT_HTIF7;
    default:
        return 0xfful; // TODO
    }

}

template< std::uintptr_t DMA_stream, uint32_t channel >
constexpr auto DMA_dev< DMA_stream, channel >::pick_TC_IF()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_IT_TCIF0;
    case 1:
        return DMA_IT_TCIF1;
    case 2:
        return DMA_IT_TCIF2;
    case 3:
        return DMA_IT_TCIF3;
    case 4:
        return DMA_IT_TCIF4;
    case 5:
        return DMA_IT_TCIF5;
    case 6:
        return DMA_IT_TCIF6;
    case 7:
        return DMA_IT_TCIF7;
    default:
        return 0xfful; // TODO
    }
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_ERR_IF()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_IT_TEIF0 | DMA_IT_DMEIF0 | DMA_IT_FEIF0;
    case 1:
        return DMA_IT_TEIF1 | DMA_IT_DMEIF1 | DMA_IT_FEIF1;
    case 2:
        return DMA_IT_TEIF2 | DMA_IT_DMEIF2 | DMA_IT_FEIF2;
    case 3:
        return DMA_IT_TEIF3 | DMA_IT_DMEIF3 | DMA_IT_FEIF3;
    case 4:
        return DMA_IT_TEIF4 | DMA_IT_DMEIF4 | DMA_IT_FEIF4;
    case 5:
        return DMA_IT_TEIF5 | DMA_IT_DMEIF5 | DMA_IT_FEIF5;
    case 6:
        return DMA_IT_TEIF6 | DMA_IT_DMEIF6 | DMA_IT_FEIF6;
    case 7:
        return DMA_IT_TEIF7 | DMA_IT_DMEIF7 | DMA_IT_FEIF7;
    default:
        return static_cast< decltype (DMA_IT_TEIF7) >(-1);
    }
}


template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_HT()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_FLAG_HTIF0;
    case 1:
        return DMA_FLAG_HTIF1;
    case 2:
        return DMA_FLAG_HTIF2;
    case 3:
        return DMA_FLAG_HTIF3;
    case 4:
        return DMA_FLAG_HTIF4;
    case 5:
        return DMA_FLAG_HTIF5;
    case 6:
        return DMA_FLAG_HTIF6;
    case 7:
        return DMA_FLAG_HTIF7;
    default:
        return 0xfful; // TODO
    }

}

template< std::uintptr_t DMA_stream, uint32_t channel >
constexpr auto DMA_dev< DMA_stream, channel >::pick_TC()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_FLAG_TCIF0;
    case 1:
        return DMA_FLAG_TCIF1;
    case 2:
        return DMA_FLAG_TCIF2;
    case 3:
        return DMA_FLAG_TCIF3;
    case 4:
        return DMA_FLAG_TCIF4;
    case 5:
        return DMA_FLAG_TCIF5;
    case 6:
        return DMA_FLAG_TCIF6;
    case 7:
        return DMA_FLAG_TCIF7;
    default:
        return 0xfful; // TODO
    }
}

template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_ERR()
{
    constexpr auto stream_no = pick_stream_no();

    switch (stream_no) {
    case 0:
        return DMA_FLAG_TEIF0;
    case 1:
        return DMA_FLAG_TEIF1;
    case 2:
        return DMA_FLAG_TEIF2;
    case 3:
        return DMA_FLAG_TEIF3;
    case 4:
        return DMA_FLAG_TEIF4;
    case 5:
        return DMA_FLAG_TEIF5;
    case 6:
        return DMA_FLAG_TEIF6;
    case 7:
        return DMA_FLAG_TEIF7;
    default:
        return static_cast< decltype (DMA_FLAG_TEIF7) >(-1);
    }
}


template< std::uintptr_t DMA_stream, uint32_t channel  >
constexpr auto DMA_dev< DMA_stream, channel >::pick_IT()
{
    constexpr auto stream = pick_stream();

    if (stream == DMA1_Stream0)      // DMA1
        return DMA1_Stream0_IRQn;
    else if (stream == DMA1_Stream1)
        return DMA1_Stream1_IRQn;
    else if (stream == DMA1_Stream2)
        return DMA1_Stream2_IRQn;
    else if (stream == DMA1_Stream3)
        return DMA1_Stream3_IRQn;
    else if (stream == DMA1_Stream4)
        return DMA1_Stream4_IRQn;
    else if (stream == DMA1_Stream5)
        return DMA1_Stream5_IRQn;
    else if (stream == DMA1_Stream6)
        return DMA1_Stream6_IRQn;
    else if (stream == DMA1_Stream7)
        return DMA1_Stream7_IRQn;
    else if (stream == DMA2_Stream0)  // DMA2
        return DMA2_Stream0_IRQn;
    else if (stream == DMA2_Stream1)
        return DMA2_Stream1_IRQn;
    else if (stream == DMA2_Stream2)
        return DMA2_Stream2_IRQn;
    else if (stream == DMA2_Stream3)
        return DMA2_Stream3_IRQn;
    else if (stream == DMA2_Stream4)
        return DMA2_Stream4_IRQn;
    else if (stream == DMA2_Stream5)
        return DMA2_Stream5_IRQn;
    else if (stream == DMA2_Stream6)
        return DMA2_Stream6_IRQn;
    else if (stream == DMA2_Stream7)
        return DMA2_Stream7_IRQn;
    else
        return static_cast< decltype (DMA1_Stream0_IRQn) >(-1);
}




#endif

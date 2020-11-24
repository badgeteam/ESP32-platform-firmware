import machine,utime,ustruct
i2c = machine.I2C(scl=machine.Pin(5),sda=machine.Pin(4),freq=400000)
sensor = i2c.scan()[0] if len(i2c.scan()) >= 1 else None

# https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf

def cmd_i2c(sensor,addr,val):
    try:
        global i2c
        return i2c.writeto_mem(sensor, addr, val)
    except:
        return None

def noisy_readfrom_mem(device, address, length, n_tries=5):
    ## Tries reading a few times due to buggy i2c bus
    for _ in range(0, n_tries):
        try:
            return i2c.readfrom_mem(device, address, length)
        except:
            pass
    raise OSError('NODEV')

def has_sensor():
    return sensor is not None

# init stuffs
def init():
    cmd_i2c(sensor,0x19,b'\x00')  # No sample rate divider
    cmd_i2c(sensor,0x1a,b'\x03')  # 44Hz bandwidth filter
    cmd_i2c(sensor,0x1b,b'\x08')  # 500 degrees per second gyro range
    cmd_i2c(sensor,0x1c,b'\x00')  # 2g of acceleration range
    cmd_i2c(sensor,0x6b,b'\x01')  # Use X-axis gyro PLL as clock source

def get_gyro():
    result = (0,0,0)
    try:
        result = ustruct.unpack(">hhh", noisy_readfrom_mem(sensor, 0x43, 6))
    except Exception as e:
        print('Got error:', e)
    finally:
        return result

def get_accel():
    x, y, z = (0,0,0)
    try:
        x, y, z = ustruct.unpack(">hhh", noisy_readfrom_mem(sensor, 0x3b, 6))
    except Exception as e:
        print('Got error:', e)
    finally:
        return (x, y, z)

def get_temp():
    return ustruct.unpack(">h",noisy_readfrom_mem(sensor,0x41,2)) [0] / 340 + 36.53


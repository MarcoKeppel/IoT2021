class Sensor():

    def __init__(self, name):
        self.name = name
        # type?
        # val_type?
        # update_rate?
        self.val = None
        # pin?

class Slave():

    def __init__(self, addr, name, sensors=None):
        self.addr = addr
        self.name = name
        if sensors is not None:
            self.sensors = self.parse_sensors(sensors)
        else:
            self.sensors = sensors

    def parse_sensors(self, sensors):
        s = []
        for i in sensors:
            print(i)
            s.append(
                Sensor("")  # should be Sensor(i.name) but the name is not currently sent
            )
        return s
    
    def set_sensors(self, sensors):
        print(sensors)
        self.sensors = self.parse_sensors(sensors)

    def __str__(self):
        s = ''
        s += 'name: {}\n'.format(self.name)
        s += '\tsensors:\n'
        for i in self.sensors:
            s += '\tname: {}\n'.format(i.name)
            s += '\t\tvalue {}\n'.format(i.val)
        return s.strip()

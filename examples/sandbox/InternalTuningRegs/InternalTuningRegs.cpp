#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include <scservo/SCServo.h>

struct RegSnapshot {
  int acc;
  int vmax;
  int amax;
  int kacc;
  int dts;
};

static std::string now_stamp()
{
  std::time_t t = std::time(nullptr);
  std::tm tm{};
  localtime_r(&t, &tm);
  char buf[64];
  std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
  return std::string(buf);
}

static int read_checked(SMS_STS &servo, int id, int addr, const char *name)
{
  int v = servo.readByte(id, addr);
  if (v < 0) {
    std::cerr << "ERROR: read " << name << " failed on ID " << id << ", sdk_err=" << servo.getErr() << std::endl;
  }
  return v;
}

static bool write_checked(SMS_STS &servo, int id, int addr, int value, const char *name)
{
  int ok = servo.writeByte((u8)id, (u8)addr, (u8)value);
  if (ok != 1) {
    std::cerr << "ERROR: write " << name << "=" << value << " failed on ID " << id << ", sdk_err=" << servo.getErr() << std::endl;
    return false;
  }
  return true;
}

static RegSnapshot read_snapshot(SMS_STS &servo, int id)
{
  RegSnapshot s{};
  s.acc = read_checked(servo, id, SMS_STS_ACC, "ACC(41)");
  s.vmax = read_checked(servo, id, SMS_STS_VMAX, "VMAX(84)");
  s.amax = read_checked(servo, id, SMS_STS_AMAX, "AMAX(85)");
  s.kacc = read_checked(servo, id, SMS_STS_KACC, "KACC(86)");
  s.dts = read_checked(servo, id, SMS_STS_DTS, "DTS(81)");
  return s;
}

static void print_snapshot(const char *label, const RegSnapshot &s)
{
  std::cout << label << std::endl;
  std::cout << "  ACC(41)=" << s.acc << std::endl;
  std::cout << "  VMAX(84)=" << s.vmax << std::endl;
  std::cout << "  AMAX(85)=" << s.amax << std::endl;
  std::cout << "  KACC(86)=" << s.kacc << std::endl;
  std::cout << "  DTS(81)=" << s.dts << std::endl;
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0]
              << " <serial_port> [baud] [motor_id] [acc] [vmax] [amax] [kacc] [dts]" << std::endl;
    std::cout << "Example: " << argv[0] << " /dev/ttyACM0 1000000 7 0 254 254 100 10" << std::endl;
    return 1;
  }

  const char *port = argv[1];
  int baud = (argc >= 3) ? std::atoi(argv[2]) : 1000000;
  int id = (argc >= 4) ? std::atoi(argv[3]) : 7;

  int target_acc = (argc >= 5) ? std::atoi(argv[4]) : 0;
  int target_vmax = (argc >= 6) ? std::atoi(argv[5]) : 254;
  int target_amax = (argc >= 7) ? std::atoi(argv[6]) : 254;
  int target_kacc = (argc >= 8) ? std::atoi(argv[7]) : 100;
  int target_dts = (argc >= 9) ? std::atoi(argv[8]) : 10;

  SMS_STS servo;
  if (!servo.begin(baud, port)) {
    std::cerr << "Failed to open " << port << " at baud " << baud << std::endl;
    return 2;
  }
  servo.IOTimeOut = 100;

  if (servo.Ping(id) == -1) {
    std::cerr << "Ping failed for ID " << id << ", sdk_err=" << servo.getErr() << std::endl;
    servo.end();
    return 3;
  }

  int mode = servo.readByte(id, SMS_STS_MODE);
  if (mode != 1) {
    std::cerr << "ERROR: motor ID " << id << " not in mode 1 (actual=" << mode << ")" << std::endl;
    servo.end();
    return 4;
  }

  RegSnapshot before = read_snapshot(servo, id);
  print_snapshot("Before write:", before);

  if (servo.unLockEeprom((u8)id) != 1) {
    std::cerr << "ERROR: unLockEeprom failed for ID " << id << std::endl;
    servo.end();
    return 5;
  }

  if (!write_checked(servo, id, SMS_STS_VMAX, target_vmax, "VMAX(84)") ||
      !write_checked(servo, id, SMS_STS_AMAX, target_amax, "AMAX(85)") ||
      !write_checked(servo, id, SMS_STS_KACC, target_kacc, "KACC(86)") ||
      !write_checked(servo, id, SMS_STS_DTS, target_dts, "DTS(81)")) {
    servo.LockEeprom((u8)id);
    servo.end();
    return 6;
  }

  if (servo.LockEeprom((u8)id) != 1) {
    std::cerr << "ERROR: LockEeprom failed for ID " << id << std::endl;
    servo.end();
    return 7;
  }

  if (!write_checked(servo, id, SMS_STS_ACC, target_acc, "ACC(41)")) {
    servo.end();
    return 8;
  }

  RegSnapshot after = read_snapshot(servo, id);
  print_snapshot("After write:", after);

  std::string stamp = now_stamp();
  std::string report_path = "/tmp/scservo_internal_tuning_" + stamp + ".txt";
  std::ofstream r(report_path);
  r << "InternalTuningRegs report\n";
  r << "port=" << port << " baud=" << baud << " id=" << id << " mode=" << mode << "\n";
  r << "targets: ACC=" << target_acc << " VMAX=" << target_vmax << " AMAX=" << target_amax
    << " KACC=" << target_kacc << " DTS=" << target_dts << "\n\n";
  r << "Before: ACC=" << before.acc << " VMAX=" << before.vmax << " AMAX=" << before.amax
    << " KACC=" << before.kacc << " DTS=" << before.dts << "\n";
  r << "After : ACC=" << after.acc << " VMAX=" << after.vmax << " AMAX=" << after.amax
    << " KACC=" << after.kacc << " DTS=" << after.dts << "\n";
  r.close();

  std::cout << "Report written to " << report_path << std::endl;
  servo.end();
  return 0;
}

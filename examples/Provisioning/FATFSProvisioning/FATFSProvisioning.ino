#include "FS.h"
#include "FFat.h"

/* This macro is used to specify the base path of the fatfs partiiton */
#define FATFS_BASE_PATH "/ffat"

/* This macro is used to format the fatfs if in case the fatfs initialization fails i.e reset if not required */
#define FORMAT_FATFS_IF_FAILED true

/* This macro is used to format the fatfs in the beginnig i.e reset if not required */
#define FORMAT_FATFS_IN_BEGINNING true  

/* This macro is used to specify the name of the device config file */
#define DEVICE_CONFIG_FILE_NAME "/device_config.json"

/* This macro is used to specify the maximum size of device config json in bytes that need to be handled for particular device */
#define DEVICE_CONFIG_STR_LENGTH 8192 

/* This macro is used to print the device config read/write buffers to serial monitor i.e set to debug any issue */
#define PRINT_BUFFERS_TO_SERIAL false

char deviceConfigReadStr[DEVICE_CONFIG_STR_LENGTH] = "";
char deviceConfigWriteStr[DEVICE_CONFIG_STR_LENGTH] = R"(
{
    "project_id": "espbytebeamsdktest",
    "broker": "cloud.bytebeam.io",
    "port": 8883,
    "device_id": "1",
    "authentication": {
        "ca_certificate": "-----BEGIN CERTIFICATE-----\nMIIFrDCCA5SgAwIBAgICB+MwDQYJKoZIhvcNAQELBQAwdzEOMAwGA1UEBhMFSW5k\naWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQHEwlCYW5nYWxvcmUxFzAVBgNV\nBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIxMDkwMjExMDYyM1oXDTMxMDkwMjExMDYyM1owdzEOMAwG\nA1UEBhMFSW5kaWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQHEwlCYW5nYWxv\ncmUxFzAVBgNVBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDAS\nBgNVBAoTC0J5dGViZWFtLmlvMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKC\nAgEAr/bnOa/8AUGZmd/s+7rejuROgeLqqU9X15KKfKOBqcoMyXsSO65UEwpzadpw\nMl7GDCdHqFTymqdnAnbhgaT1PoIFhOG64y7UiNgiWmbh0XJj8G6oLrW9rQ1gug1Q\n/D7x2fUnza71aixiwEL+KsIFYIdDuzmoRD3rSer/bKOcGGs0WfB54KqIVVZ1DwsU\nk1wx5ExsKo7gAdXMAbdHRI2Szmn5MsZwGL6V0LfsKLE8ms2qlZe50oo2woLNN6XP\nRfRL4bwwkdsCqXWkkt4eUSNDq9hJsuINHdhO3GUieLsKLJGWJ0lq6si74t75rIKb\nvvsFEQ9mnAVS+iuUUsSjHPJIMnn/J64Nmgl/R/8FP5TUgUrHvHXKQkJ9h/a7+3tS\nlV2KMsFksXaFrGEByGIJ7yR4qu9hx5MXf8pf8EGEwOW/H3CdWcC2MvJ11PVpceUJ\neDVwE7B4gPM9Kx02RNwvUMH2FmYqkXX2DrrHQGQuq+6VRoN3rEdmGPqnONJEPeOw\nZzcGDVXKWZtd7UCbcZKdn0RYmVtI/OB5OW8IRoXFYgGB3IWP796dsXIwbJSqRb9m\nylICGOceQy3VR+8+BHkQLj5/ZKTe+AA3Ktk9UADvxRiWKGcejSA/LvyT8qzz0dqn\nGtcHYJuhJ/XpkHtB0PykB5WtxFjx3G/osbZfrNflcQZ9h1MCAwEAAaNCMEAwDgYD\nVR0PAQH/BAQDAgKEMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFKl/MTbLrZ0g\nurneOmAfBHO+LHz+MA0GCSqGSIb3DQEBCwUAA4ICAQAlus/uKic5sgo1d2hBJ0Ak\ns1XJsA2jz+OEdshQHmCCmzFir3IRSuVRmDBaBGlJDHCELqYxKn6dl/sKGwoqoAQ5\nOeR2sey3Nmdyw2k2JTDx58HnApZKAVir7BDxbIbbHmfhJk4ljeUBbertNXWbRHVr\ncs4XBNwXvX+noZjQzmXXK89YBsV2DCrGRAUeZ4hQEqV7XC0VKmlzEmfkr1nibDr5\nqwbI+7QWIAnkHggYi27lL2UTHpbsy9AnlrRMe73upiuLO7TvkwYC4TyDaoQ2ZRpG\nHY+mxXLdftoMv/ZvmyjOPYeTRQbfPqoRqcM6XOPXwSw9B6YddwmnkI7ohNOvAVfD\nwGptUc5OodgFQc3waRljX1q2lawZCTh58IUf32CRtOEL2RIz4VpUrNF/0E2vts1f\npO7V1vY2Qin998Nwqkxdsll0GLtEEE9hUyvk1F8U+fgjJ3Rjn4BxnCN4oCrdJOMa\nJCaysaHV7EEIMqrYP4jH6RzQzOXLd0m9NaL8A/Y9z2a96fwpZZU/fEEOH71t3Eo3\nV/CKlysiALMtsHfZDwHNpa6g0NQNGN5IRl/w1TS1izzjzgWhR6r8wX8OPLRzhNRz\n2HDbTXGYsem0ihC0B8uzujOhTHcBwsfxZUMpGjg8iycJlfpPDWBdw8qrGu8LeNux\na0cIevjvYAtVysoXInV0kg==\n-----END CERTIFICATE-----\n",
        "device_certificate": "-----BEGIN CERTIFICATE-----\nMIIEcjCCAlqgAwIBAgICB+MwDQYJKoZIhvcNAQELBQAwdzEOMAwGA1UEBhMFSW5k\naWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQHEwlCYW5nYWxvcmUxFzAVBgNV\nBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIyMTEwMzA3NDcwMloXDTMyMTEwMzA3NDcwMlowKTEbMBkG\nA1UEChMSZXNwYnl0ZWJlYW1zZGt0ZXN0MQowCAYDVQQDEwExMIIBIjANBgkqhkiG\n9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzsm8CF8fKSzWFKZeO52RJRGER4z7sYrgTb9x\noIeljE8kmhR+EW217sxUyUn+rmJSCxRz6jtspKv37POPaQilmwtriVq8L46YCrXa\nbI9manlqw0WqAfaWAlFXzM13CuXeGZKhuAK3INs15jqP0s+oWFvww8+hww540I+s\nn6j5yEUnWKSIgoYEc82cwvj0qOvaxYxjUTXgV2IDLSXdFJbZq4k9LDbzPZdDe0aY\n4X9UpdrjS5cRWT3Ok4VwWpMRwOT654CTwE8WUja/pHTRFbMYkqd1jrD2Joqtm+u9\nplTq7I9/fDnkRqPjueDQ04LKTbUC2UsRKw1VmHPndw22+mDPUwIDAQABo1YwVDAO\nBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwIwHwYDVR0jBBgwFoAU\nqX8xNsutnSC6ud46YB8Ec74sfP4wDAYDVR0RBAUwA4IBMTANBgkqhkiG9w0BAQsF\nAAOCAgEAcj5Ytt47AdmsXBsv/K7YkUvkNwV64luBMQtqmTKRDWnIQLaFzPjdpieu\n81ktFKklKw6pReWdUPSBy6hKq3zzGvSsShBSghbOAJvlPkK6jpLy5TzZdIj8ug7g\nMCzZvqdmznekSg4elGHJIYUfHD6ZUJaEkkGD1yQpxOIVCnodhN2246SXJK23itmz\nB5y1AHU5zydN7Ys0oY1l8RCc5Iz24IcQAi//2pIPe9ORks3QBBi+qY9m+emHCqgb\n48hNe5v7X+yF4VITlWs3Mfk0gSgbDQ4OwVf/8/D7o0hQ8EU5bzXNvf49Kd2mWTii\nQdQDdNLxZsdgWNwYdqrQhA0qN02yJxkhNm5GXmP75zmg0bPe4I+pNwuy+B32wFZn\nNcZrh4HZzS1cAyyyr3E59zsPjqcohaUcBpe6oUoQ8z1V7K9FR31TnFfvjg9mbDnX\nv2+PXsp0m7f4e1QfepZxUwyjemxOe/b+IEc7PckIrkPy46JaIvi9ONrTR8Y5wDir\nbGuzpmiyMFlyzK4ULh7kt7MU/jU/ulnqDU8JvAnhhxK3kgV4vv8LSYJSEYYP9i1S\nj/EPPQBANdtr4B2pcV1ig13dinq7rir6Q/pnkKrAFOfZcvOWW2BvPOC2dcnCYJpB\n1R5QJGXqna8Uh+ZwThs0K/Nd9dml1w/Rug8hx0VHoIe6AjXEepQ=\n-----END CERTIFICATE-----\n",
        "device_private_key": "-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEAzsm8CF8fKSzWFKZeO52RJRGER4z7sYrgTb9xoIeljE8kmhR+\nEW217sxUyUn+rmJSCxRz6jtspKv37POPaQilmwtriVq8L46YCrXabI9manlqw0Wq\nAfaWAlFXzM13CuXeGZKhuAK3INs15jqP0s+oWFvww8+hww540I+sn6j5yEUnWKSI\ngoYEc82cwvj0qOvaxYxjUTXgV2IDLSXdFJbZq4k9LDbzPZdDe0aY4X9UpdrjS5cR\nWT3Ok4VwWpMRwOT654CTwE8WUja/pHTRFbMYkqd1jrD2Joqtm+u9plTq7I9/fDnk\nRqPjueDQ04LKTbUC2UsRKw1VmHPndw22+mDPUwIDAQABAoIBAHv3A/ogzBVrA4ut\nkKA8fV6zeZFLOzfcAUuakQujReMvLsoPruPT2VUmuU1SRpNT7csmn7azmRW+4gny\nmO5meKDR382fz2DTIuKI0kByVvtNfmtBwAEdSiBpkzD7m3m1A8hg1wHw3sebolw6\njy3Zvxn5RASe3GKKsnKVLu8n5VXgyytn2I6sYOmKoLv3N9V3tziHL3Gl2AfWAd5P\nJHJdxZ50hhjzUnI0MmvUU6M9G26xagxNNibzOWv3BEuBRw5fuVpt/QoBouFhcKF7\nEdCFpiZ8LJFpux6FyLDOOAa7mj+Y6yK1qKsJbFGG8AnhPTO/bAc1m7yQcJygH9cl\nretVDcECgYEA4z+NkjqIPRiAvOsmJD6KfMeABkZc4JmART+KtrpoJmPWc2WDxu8M\ngP1eqmh/aGGORY8hcFyl+SIf5JvP13Dd/47tSK4vhyEGnnqRt+wjjuzci0rDvFdH\ntaUbjlb4kKStssQPTTmG726Zd1f32eKzBnCY8YqWdMMM1HXak20rPo8CgYEA6PN8\nYIcIrRkFzyvam8m5pCeoHylBgSNjN2j8dGC8b6SGMG6y4+rUiuxfhC/Oq7UtOvf+\njYQQgeMjB22dhcvJa6rjuuKXXpbcgNnzcemnySohUr7Nm1bXHwRfoqMAmeh3pkcL\nL+K4Mf6Gd1i2hiYd4Dd59m5/p3W+QUyKa4iaRP0CgYAf2/UZHyOijSDfW4hJZIs9\n2ypTtuGmi160Vqg33gJj/3M9UmobJcB3BQ6UjXnvRF4R2nMxsYuDVglqn32QEr7M\n6VjS67i2FSc8aKqtQmnpy8NPs/elHAdtq+wlFIRcovnHKj2K8hm8z6CsXqTc4y9+\nI6MNmgRl8kKGNs+iA5ggeQKBgBKLN25zsWQeJtE8G3XlVArWQVLhtN4z0/UYPWiC\nPt3gSfJXDZSJIAxDDsN2DsyqaoRUM4ZOagX878/qkOySsWEJxIEfAo+8EKeNMgzy\nXbHs0aRFnhZsjklgzsAim6ykzcmFxEU2lhUcvtWHUVhSdnRf1iyg1Taeb9vA3Q/8\nWtN9AoGBAN+rik+pvcdPAexngqmsUgoW8Abvv7dyBmt99KfnyRDfbt6EM68CZI90\naOlNE9P7XZyOyfQdzOlJWv8B78dwrbwMiwkhGyb6ms11GGB5J90f8oON4tMjEzdW\nKFI0T+ZOXN99GcKLHeg5GfGqHcHlEhXP9BtRyTJVz914alvOIaiY\n-----END RSA PRIVATE KEY-----\n"
    }
}
)";

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory : %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print(" - DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print(" - FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char *path, char *message) {
  Serial.printf("Reading file : %s\r\n", path);

  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  char chr = ' ';
  int strIndex = 0;

  Serial.println("- read from file");
  while (file.available()) {
    chr = file.read();
    message[strIndex++] = chr;
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file : %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

#if FORMAT_FATFS_IN_BEGINNING
  if (!FFat.format()) {
    Serial.println("fatfs format failed");
    return;
  } else {
    Serial.println("fatfs format success");
  }
#endif

  // initalize the fatfs file system 
  if (!FFat.begin(FORMAT_FATFS_IF_FAILED, FATFS_BASE_PATH)) {  
    Serial.println("fatfs mount failed");
    return;
  } else {
    Serial.println("fatfs mount success");
  }

  listDir(FFat, "/", 0);                                           //  list the directories in the fatfs
  writeFile(FFat, DEVICE_CONFIG_FILE_NAME, deviceConfigWriteStr);  //  write the device configuration string to fatfs
  readFile(FFat, DEVICE_CONFIG_FILE_NAME, deviceConfigReadStr);    //  read the device configuration string from fatfs

  // de-initalize the fatfs file system 
  FFat.end();

#if PRINT_BUFFERS_TO_SERIAL
  Serial.println();
  Serial.println("deviceConfigWriteStr : ");
  Serial.println(deviceConfigWriteStr);
  Serial.println("deviceConfigReadStr : ");
  Serial.println(deviceConfigReadStr);
#endif

  // verify the write process and log the result
  if (!memcmp(deviceConfigReadStr, deviceConfigWriteStr, DEVICE_CONFIG_STR_LENGTH)) {  
    Serial.println("device provisioning success !");
  } else {
    Serial.println("device provisioning failed !");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  // nothing to do here
}
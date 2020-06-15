// ***********************************************
// * InfluxDB Server Access Settings
// *
// ***********************************************
char influx_user[] = "<user>";
char influx_pssw[] = "<user password>";

char influx_url[] = "FQDN or IP Address <http://xxx.xxx.xxx.xxx:8086";
char influx_dbase[] = "<influxdb Name>";

//
// InfluxDB line protocol is a text based format for writing points to InfluxDB.
// Line protocol syntax
// <measurement>[,<tag_key>=<tag_value>[,<tag_key>=<tag_value>]] <field_key>=<field_value>[,<field_key>=<field_value>] [<timestamp>]
//
char influx_measurement[] = "Node-01";          // Measurement Name
char influx_tagkey1[] = "ESP8266";              // Data Point <oject>.addTag("tag_key", tag_value);
char influx_tagkey2[] = "BME280";
char influx_fieldkey1[] = "Temperature";        // Data Point <oject>.addField("field_key", field_value);
char influx_fieldkey2[] = "DewPoimt";
char influx_fieldkey3[] = "HeatIndex";
char influx_fieldkey4[] = "Humidity";
char influx_fieldkey5[] = "Pressure";
char influx_fieldkey6[] = "ADCValue";    
char influx_fieldkey7[] = "Voltage";
char influx_fieldkey8[] = "Percentage";
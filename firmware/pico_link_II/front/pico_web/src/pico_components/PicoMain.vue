<script setup>
import { ref, reactive, computed } from "vue";
import PicoInputNumPar from "./PicoInputNumPar.vue";
import PicoInputStrPar from "./PicoInputStrPar.vue";
import PicoSelectPar from "./PicoSelectPar.vue";
import PicoGetButton from "./PicoGetButton.vue";
import PicoPostButton from "./PicoPostButton.vue";
import PicoResetButton from "./PicoResetButton.vue";

const IN_METHOD_UART = 0;
const IN_METHOD_SPI = 1;

const OUT_METHOD_UDP = 0;
const OUT_METHOD_TCP = 1;

const WORK_MODE_CLIENT = 0;
const WORK_MODE_SERVER = 1;

const WIFI_MODE_STA = 0;
const WIFI_MODE_AP = 1;

const inMethodOptions = ref([
  { text: 'UART', value: '0' },
  { text: 'SPI', value: '1' }
]);
const uartDataBitOptions = ref([
  { text: '5bit', value: '0' },
  { text: '6bit', value: '1' },
  { text: '7bit', value: '2' },
  { text: '8bit', value: '3' }
]);
const uartParityOptions = ref([
  { text: '无', value: '0' },
  { text: '偶校验', value: '1' },
  { text: '奇校验', value: '2' }
]);
const uartStopBitOptions = ref([
  { text: '1bit', value: '0' },
  { text: '1.5bit', value: '1' },
  { text: '2bit', value: '2' }
]);
const spiClkModeOptions = ref([
  { text: '0', value: '0' },
  { text: '1', value: '1' },
  { text: '2', value: '2' },
  { text: '3', value: '3' }
]);
const spiBusModeOptions = ref([
  { text: '全双工1bit', value: '0' },
  { text: '半双工1/2bit', value: '1' }
]);
const outMethodOptions = ref([
  { text: 'UDP', value: '0' },
  { text: 'TCP', value: '1' }
]);
const wifiModeOptions = ref([
  { text: 'STA', value: '0' },
  { text: 'AP', value: '1' }
]);
const wifiApMaxConnOptions = ref([
  { text: '1', value: '1' },
  { text: '2', value: '2' },
  { text: '3', value: '3' },
  { text: '4', value: '4' }
]);
const wifiApChannelOptions = ref([
  { text: '1', value: '1' },
  { text: '2', value: '2' },
  { text: '3', value: '3' },
  { text: '4', value: '4' },
  { text: '5', value: '5' },
  { text: '6', value: '6' },
  { text: '7', value: '7' },
  { text: '8', value: '8' },
  { text: '9', value: '9' },
  { text: '10', value: '10' },
  { text: '11', value: '11' },
  { text: '12', value: '12' },
  { text: '13', value: '13' },
]);
const twoWayEnOptions = ref([
  { text: '否', value: '0' },
  { text: '是', value: '1' }
]);
const workModeOptions = ref([
  { text: 'client', value: '0' },
  { text: 'server', value: '1' }
]);

const inMethod = reactive({ data: ref("") });
const uartRate = reactive({ data: ref(""), isValid: ref(false) });
const uartDataBit = reactive({ data: ref("") });
const uartParity = reactive({ data: ref("") });
const uartStopBit = reactive({ data: ref("") });
const spiClkMode = reactive({ data: ref("") });
const spiBusMode = reactive({ data: ref("") });
const outMethod = reactive({ data: ref("") });
const wifiMode = reactive({ data: ref("") });
const wifiApMaxConn = reactive({ data: ref("") });
const wifiApChannel = reactive({ data: ref("") });
const STA_SSID = reactive({ data: ref(""), isValid: ref(false) });
const STA_PWD = reactive({ data: ref(""), isValid: ref(false) });
const AP_SSID = reactive({ data: ref(""), isValid: ref(false) });
const AP_PWD = reactive({ data: ref(""), isValid: ref(false) });
const twoWayEn = reactive({ data: ref("") });
const workMode = reactive({ data: ref("") });
const remoteIP = reactive({ data: ref(""), isValid: ref(false) });
const remotePort = reactive({ data: ref(""), isValid: ref(false) });
const localGW = reactive({ data: ref(""), isValid: ref(false) });
const localNetmask = reactive({ data: ref(""), isValid: ref(false) });
const localIP = reactive({ data: ref(""), isValid: ref(false) });
const localPort = reactive({ data: ref(""), isValid: ref(false) });

const postDataValid = computed({
  get() {
    return (uartRate.isValid &&
      STA_SSID.isValid &&
      STA_PWD.isValid &&
      AP_SSID.isValid &&
      AP_PWD.isValid &&
      remoteIP.isValid &&
      remotePort.isValid &&
      localGW.isValid &&
      localNetmask.isValid &&
      localIP.isValid &&
      localPort.isValid
    );
  },
  set(newValue) {
    uartRate.isValid = newValue;
    STA_SSID.isValid = newValue;
    STA_PWD.isValid = newValue;
    AP_SSID.isValid = newValue;
    AP_PWD.isValid = newValue;
    remoteIP.isValid = newValue;
    remotePort.isValid = newValue;
    localGW.isValid = newValue;
    localNetmask.isValid = newValue;
    localIP.isValid = newValue;
    localPort.isValid = newValue;
  }
});

const postData = computed({
  get() {
    const uartConfig = {
      "uart_rate": parseInt(uartRate.data),
      "uart_data_bit": parseInt(uartDataBit.data),
      "uart_parity": parseInt(uartParity.data),
      "uart_stop_bit": parseInt(uartStopBit.data)
    };
    const dataList = {
      "in_method": parseInt(inMethod.data),
      "uart_config": uartConfig,
      "spi_clk_mode": parseInt(spiClkMode.data),
      "spi_bus_mode": parseInt(spiBusMode.data),
      "out_method": parseInt(outMethod.data),
      "wifi_mode": parseInt(wifiMode.data),
      "wifi_ap_max_conn": parseInt(wifiApMaxConn.data),
      "wifi_ap_channel": parseInt(wifiApChannel.data),
      "STA_SSID": STA_SSID.data,
      "STA_PWD": STA_PWD.data,
      "AP_SSID": AP_SSID.data,
      "AP_PWD": AP_PWD.data,
      "two_way_en": parseInt(twoWayEn.data),
      "work_mode": parseInt(workMode.data),
      "remote_ip": remoteIP.data,
      "remote_port": parseInt(remotePort.data),
      "local_gw": localGW.data,
      "local_netmask": localNetmask.data,
      "local_ip": localIP.data,
      "local_port": parseInt(localPort.data),
    };
    return dataList;
  },
  set(newValue) {
    inMethod.data = newValue["in_method"].toString()
    uartRate.data = newValue["uart_config"]["uart_rate"].toString()
    uartDataBit.data = newValue["uart_config"]["uart_data_bit"].toString()
    uartParity.data = newValue["uart_config"]["uart_parity"].toString()
    uartStopBit.data = newValue["uart_config"]["uart_stop_bit"].toString()
    spiClkMode.data = newValue["spi_clk_mode"].toString()
    spiBusMode.data = newValue["spi_bus_mode"].toString()
    outMethod.data = newValue["out_method"].toString()
    wifiMode.data = newValue["wifi_mode"].toString()
    wifiApMaxConn.data = newValue["wifi_ap_max_conn"].toString()
    wifiApChannel.data = newValue["wifi_ap_channel"].toString()
    STA_SSID.data = newValue["STA_SSID"]
    STA_PWD.data = newValue["STA_PWD"]
    AP_SSID.data = newValue["AP_SSID"]
    AP_PWD.data = newValue["AP_PWD"]
    twoWayEn.data = newValue["two_way_en"].toString()
    workMode.data = newValue["work_mode"].toString()
    remoteIP.data = newValue["remote_ip"]
    remotePort.data = newValue["remote_port"].toString()
    localGW.data = newValue["local_gw"]
    localNetmask.data = newValue["local_netmask"]
    localIP.data = newValue["local_ip"]
    localPort.data = newValue["local_port"].toString()
  }
});

const isGetConfig = ref(false);

</script>

<template>
  <!-- <p>postDatavalid: {{ postDataValid }}</p> -->
  <!-- <p>postData: {{ postData }}</p><br> -->
  <hr>
  <PicoSelectPar label-name="输入方式" :options="inMethodOptions" v-model:inputValue="inMethod.data"
    :custom-style="{ fontWeight: 'bold' }" />
  <hr>
  <Transition appear name="slide-fade" mode="out-in">
    <div v-if="inMethod.data == IN_METHOD_UART">
      <PicoInputNumPar label-name="UART波特率" :range="[300, 5000000]" v-model:inputValue="uartRate.data"
        v-model:inputValid="uartRate.isValid" />
      <PicoSelectPar label-name="UART数据位" :options="uartDataBitOptions" v-model:inputValue="uartDataBit.data" />
      <PicoSelectPar label-name="UART校验位" :options="uartParityOptions" v-model:inputValue="uartParity.data" />
      <PicoSelectPar label-name="UART停止位" :options="uartStopBitOptions" v-model:inputValue="uartStopBit.data" />
    </div>
    <div v-else-if="inMethod.data == IN_METHOD_SPI">
      <PicoSelectPar label-name="SPI时钟模式" :options="spiClkModeOptions" v-model:inputValue="spiClkMode.data" />
      <PicoSelectPar label-name="SPI数据模式" :options="spiBusModeOptions" v-model:inputValue="spiBusMode.data" />
    </div>
  </Transition>
  <hr>
  <PicoSelectPar label-name="输出方式" :options="outMethodOptions" v-model:inputValue="outMethod.data"
    :custom-style="{ fontWeight: 'bold' }" />
  <hr>
  <PicoSelectPar label-name="WiFi连接方式" :options="wifiModeOptions" v-model:inputValue="wifiMode.data" />
  <Transition name="slide-fade" mode="out-in">
    <div v-if="wifiMode.data == WIFI_MODE_STA">
      <PicoInputStrPar label-name="STA连接的WiFi名称" str-type="SSID" :max-length="32" v-model:inputValue="STA_SSID.data"
        v-model:inputValid="STA_SSID.isValid" />
      <PicoInputStrPar label-name="STA连接的WiFi密码" str-type="PWD" :max-length="64" v-model:inputValue="STA_PWD.data"
        v-model:inputValid="STA_PWD.isValid" />
    </div>
    <div v-else-if="wifiMode.data == WIFI_MODE_AP">
      <PicoInputStrPar label-name="AP创建的WiFi名称" str-type="SSID" :max-length="32" v-model:inputValue="AP_SSID.data"
        v-model:inputValid="AP_SSID.isValid" />
      <PicoInputStrPar label-name="AP创建的WiFi密码" str-type="PWD" :max-length="64" v-model:inputValue="AP_PWD.data"
        v-model:inputValid="AP_PWD.isValid" />
      <PicoInputStrPar label-name="本地网关 IPv4地址" str-type="GW" :max-length="16" v-model:inputValue="localGW.data"
        v-model:inputValid="localGW.isValid" />
      <PicoInputStrPar label-name="子网掩码" str-type="netmask" :max-length="16" v-model:inputValue="localNetmask.data"
        v-model:inputValid="localNetmask.isValid" />
      <PicoInputStrPar label-name="本地主机 IPv4地址" str-type="IPv4" :max-length="16" v-model:inputValue="localIP.data"
        v-model:inputValid="localIP.isValid" />
      <PicoSelectPar label-name="最大连接设备数" :options="wifiApMaxConnOptions" v-model:inputValue="wifiApMaxConn.data" />
      <PicoSelectPar label-name="WiFi信道" :options="wifiApChannelOptions" v-model:inputValue="wifiApChannel.data" />
    </div>
  </Transition>
  <hr>
  <PicoSelectPar label-name="双向通信" :options="twoWayEnOptions" v-model:inputValue="twoWayEn.data" />
  <hr>
  <PicoSelectPar label-name="工作方式" :options="workModeOptions" v-model:inputValue="workMode.data" />
  <Transition name="slide-fade" mode="out-in">
    <div v-if="workMode.data == WORK_MODE_CLIENT">
      <PicoInputStrPar label-name="远程主机 IPv4地址" str-type="IPv4" :max-length="16" v-model:inputValue="remoteIP.data"
        v-model:inputValid="remoteIP.isValid" />
      <PicoInputNumPar label-name="远程主机端口" :range="[0, 65535]" v-model:inputValue="remotePort.data"
        v-model:inputValid="remotePort.isValid" />
      <hr>
    </div>
    <div v-else-if="workMode.data == WORK_MODE_SERVER">
      <PicoInputNumPar label-name="本地主机端口" :range="[0, 65535]" v-model:inputValue="localPort.data"
        v-model:inputValid="localPort.isValid" />
      <hr>
    </div>
  </Transition>
  <br>
  <div>
    <PicoGetButton v-model:allData="postData" v-model:allDataValid="postDataValid" v-model:isGetConfig="isGetConfig" />
  </div>
  <br>
  <Transition appear name="slide-fade">
    <div v-if="isGetConfig">
      <PicoPostButton :all-data="postData" :all-data-valid="postDataValid" />
      <br>
      <PicoResetButton v-model:allData="postData" v-model:allDataValid="postDataValid" />
    </div>
  </Transition>
</template>


<style scoped>
.slide-fade-enter-active {
  transition: all 0.3s ease-out;
}

.slide-fade-leave-active {
  transition: all 0.3s cubic-bezier(1, 0.5, 0.8, 1);
}

.slide-fade-enter-from,
.slide-fade-leave-to {
  transform: translateX(20px);
  opacity: 0;
}
</style>
<script setup>
import { ref, reactive, computed } from "vue"
defineProps({
  labelName: { type: String, required: true },
  strType: { type: String, required: true },
  maxLength: { type: Number, required: true },
  inputValue: { type: String, required: true },
  inputValid: { type: Boolean, required: true },
})
const emit = defineEmits(["update:inputValue", "update:inputValid"])
const checkResult = ref("")

function checkStr(value, type) {
  if (value.length > 0) {
    if (type == "SSID") {
      emit('update:inputValid', true)
    }
    else if (type == "PWD") {
      if (getBytesLength(value) < 8) {
        checkResult.value = "字节长度至少8位!"
        emit('update:inputValid', false)
      }
      else {
        emit('update:inputValid', true)
      }
    }
    else if (type == "IPv4") {
      if (!validIPAddress(value)) {
        checkResult.value = "不是合法的IPv4地址!"
        emit('update:inputValid', false)
      }
      else {
        emit('update:inputValid', true)
      }
    }
    else if (type == "GW") {
      if (!validIPAddress(value)) {
        checkResult.value = "不是合法的网关地址!"
        emit('update:inputValid', false)
      }
      else {
        emit('update:inputValid', true)
      }
    }
    else if (type == "netmask") {
      if (!validIPAddress(value)) {
        checkResult.value = "不是合法的子网掩码!"
        emit('update:inputValid', false)
      }
      else {
        emit('update:inputValid', true)
      }
    }
  }
  else {
    checkResult.value = "不能为空!"
    emit('update:inputValid', false)
  }
  emit('update:inputValue', value)
}

function getBytesLength(str) {
  var len = str.length;
  var bytes = len;
  for (var i = 0; i < len; i++) {
    if (str.charCodeAt(i) > 255) {
      bytes++;
    }
  }
  return bytes;
}

function validIPAddress(queryIP) {
  if (queryIP.indexOf('.') >= 0) {
    // IPv4
    let last = -1;
    for (let i = 0; i < 4; ++i) {
      const cur = (i === 3 ? queryIP.length : queryIP.indexOf('.', last + 1));
      if (cur < 0) {
        return false;
      }
      if (cur - last - 1 < 1 || cur - last - 1 > 3) {
        return false;
      }
      let addr = 0;
      for (let j = last + 1; j < cur; ++j) {
        if (!isDigit(queryIP[j])) {
          return false;
        }
        addr = addr * 10 + (queryIP[j].charCodeAt() - '0'.charCodeAt());
      }
      if (addr > 255) {
        return false;
      }
      if (addr > 0 && queryIP[last + 1].charCodeAt() === '0'.charCodeAt()) {
        return false;
      }
      if (addr === 0 && cur - last - 1 > 1) {
        return false;
      }
      last = cur;
    }
    return true;
  } else {
    return false;
  }
};

const isDigit = (ch) => {
  return parseFloat(ch).toString() === "NaN" ? false : true;
}
</script>

<template>
  <p>
    <label>{{ labelName }}: </label>
    <input :value="inputValue" :maxlength="maxLength" placeholder="请输入" @input="checkStr($event.target.value, strType)" />
    <label :style="{ color: 'red', fontWeight: 'bold' }" v-show="!inputValid">{{ checkResult }}</label>
    <label :style="{ color: 'blue', fontWeight: 'bold' }" v-show="inputValid"> √</label>
  </p>
</template>
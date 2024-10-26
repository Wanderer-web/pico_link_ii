<script setup>
import { ref, reactive, computed } from "vue"
defineProps({
  labelName: { type: String, required: true },
  range: { type: Array, required: true },
  inputValue: { type: String, required: true },
  inputValid: { type: Boolean, required: true },
})
const emit = defineEmits(["update:inputValue", "update:inputValid"])
const checkResult = ref("")

function isNum(value) {
  return !isNaN(parseFloat(value)) && isFinite(value);
}

function checkNumber(value, range) {
  if (value.length > 0) {
    if (!isNum(value)) {
      checkResult.value = "不是数字!"
      emit('update:inputValid', false)
    }
    else {
      var num = parseInt(value)
      if (num < range[0] || num > range[1]) {
        checkResult.value = "范围应在" + range[0] + "到" + range[1] + "之间!"
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
</script>

<template>
  <p>
    <label>{{ labelName }}: </label>
    <input :value="inputValue" placeholder="请输入" @input="checkNumber($event.target.value, range)" />
    <label :style="{ color: 'red', fontWeight: 'bold' }" v-show="!inputValid">{{ checkResult }}</label>
    <label :style="{ color: 'blue', fontWeight: 'bold' }" v-show="inputValid"> √</label>
  </p>
</template>
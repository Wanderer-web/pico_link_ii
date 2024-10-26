<script setup>
import { ref, reactive, computed } from "vue"
defineProps({
    labelName: { type: String, required: true },
    options: { type: Array, required: true },
    inputValue: { type: String, required: true },
    customStyle: { type: Object, required: false },
})
const emit = defineEmits(["update:inputValue"])
function checkSelect(value) {
    emit('update:inputValue', value)
}
</script>

<template>
    <p>
        <label :style="customStyle">{{ labelName }}: </label>
        <select :value="inputValue" @change="checkSelect($event.target.value)">
            <option :value="-1" disabled selected>请选择</option>
            <option v-for="option in options" :value="option.value">{{ option.text }}</option>
        </select>
    </p>
</template>
<script setup>
import { ref, reactive, computed } from "vue"
defineProps({
    allData: { type: Object, required: true },
    allDataValid: { type: Boolean, required: true },
    isGetConfig: { type: Boolean, required: true },
})
const emit = defineEmits(["update:allData", "update:allDataValid", "update:isGetConfig"])

function getConfig() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/api/get_config", true);
    xhttp.onreadystatechange = function () {
        if (xhttp.readyState == XMLHttpRequest.DONE) {
            if (xhttp.status == 200) {
                var responseText = xhttp.responseText;//返回结果
                if (responseText.length > 0) {
                    try {
                        var obj = JSON.parse(responseText);
                        alert("查询成功!");
                        emit('update:allData', obj)
                        emit('update:allDataValid', true)
                        emit('update:isGetConfig', true)
                    }
                    catch (err) {
                        alert("收到不合法的数据!")
                    }
                }
            }
            else if (xhttp.status == 0) {
                alert("Server closed the connection abruptly!");
                location.reload();
            } else {
                alert(xhttp.status + " Error!\n" + xhttp.responseText);
                location.reload();
            }
        }
    };
    xhttp.send(null);
}

</script>

<template>
    <p style="text-align:center">
        <button @click="getConfig">查询配置信息</button>
    </p>
</template>
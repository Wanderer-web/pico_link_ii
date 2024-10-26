<script setup>
import { ref, reactive, computed } from "vue"
defineProps({
    allData: { type: Object, required: true },
    allDataValid: { type: Boolean, required: true },
})

function sendConfig(isValid, data) {
    if (!isValid) {
        alert("参数填写不对!");
        return;
    }
    if (confirm("确认提交配置信息?") == true) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("POST", "/api/post_config", true);
        xhttp.onreadystatechange = function () {
            if (xhttp.readyState == XMLHttpRequest.DONE) {
                if (xhttp.status == 200) {
                    var responseText = xhttp.responseText.toString()//返回结果
                    if (responseText == "success") {
                        alert("提交成功！");
                    }
                } else if (xhttp.status == 0) {
                    alert("Server closed the connection abruptly!");
                    location.reload();
                } else {
                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
                    location.reload();
                }
            }
        };
        xhttp.send(JSON.stringify(data));
    }
}

</script>

<template>
    <p style="text-align:center">
        <button @click="sendConfig(allDataValid, allData)">提交配置信息</button>
    </p>
</template>
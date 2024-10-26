<script setup>
import PicoHelp from './PicoHelp.vue';
import PicoAbout from './PicoAbout.vue';
</script>

<template>
    <h3 :style="{
        fontWeight: 'bold', textAlign: 'center'
    }">智能车竞赛的小伙伴们, 垅牛电子祝您比赛成功, 发车🚗顺利🚀</h3>
    <PicoHelp></PicoHelp>
    <br>
    <PicoAbout></PicoAbout>
</template>

<style scoped>
h3 {
    font-size: 1.2rem;
    font-weight: 500;
    margin-bottom: 0.4rem;
    color: var(--color-heading);
}
</style>
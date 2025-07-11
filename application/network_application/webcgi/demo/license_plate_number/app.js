const plateNumberInput = document.querySelector('.prompt.page .lpn-form .number');
const clearButton = document.querySelector('.prompt.page .lpn-form .actions .clear');
const submitButton = document.querySelector('.prompt.page .lpn-form .actions .submit');

const setLPNInput = (number = '0000') => plateNumberInput.value = number;
const getLPNInput = () => plateNumberInput.value;
const clearLPNInput = () => setLPNInput('');

const sendChar = (c = '1') => {
  const prevLPN = getLPNInput();
  if (prevLPN.length >= 4) {
    return;
  }

  const newLPN = prevLPN + c;
  setLPNInput(newLPN);
  submitButton.disabled = newLPN.length != 4;
  return newLPN;
};

const drawLicensePlate = (number) => {
  document.querySelector('.result.page .plate').innerHTML = /*html*/`
  <img src="./assets/${number[0]}.gif" class="digit">
  <img src="./assets/${number[1]}.gif" class="digit">
  <img src="./assets/${number[2]}.gif" class="digit">
  <img src="./assets/${number[3]}.gif" class="digit">
  `;
};

const clear = () => {
  submitButton.disabled = true;
};

const submit = () => {
  const number = getLPNInput() || '0000';
  clearLPNInput();
  submitButton.disabled = true;
  drawLicensePlate(number);
  location.hash = '#p03';
};

document.querySelectorAll('.prompt.page .lpn-form .numpad .numkey').forEach(el =>
  el.addEventListener('click', () => sendChar(el.value))
);
clearButton.addEventListener('click', clear);
submitButton.addEventListener('click', submit);

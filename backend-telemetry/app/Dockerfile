FROM node:20-alpine

RUN apk add --no-cache tzdata

WORKDIR /app
COPY package.json /app/package.json
RUN npm install --production

COPY server.js /app/server.js
COPY db.js /app/db.js
COPY public /app/public

ENV PORT=8080
ENV MQTT_HOST=mqtt
ENV MQTT_PORT=1883
ENV MONGO_URI=mongodb://mongo:27017
ENV MONGO_DB=dc-monitor
ENV TZ=America/Sao_Paulo

EXPOSE 8080
CMD ["node", "server.js"]

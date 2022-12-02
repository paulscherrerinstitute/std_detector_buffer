FROM node:lts

WORKDIR /app/std-daq-doc

EXPOSE 3000 35729
COPY ./docs /app/docs
COPY ./std-daq-doc /app/std-daq-doc
RUN yarn install

CMD ["yarn", "start"]
